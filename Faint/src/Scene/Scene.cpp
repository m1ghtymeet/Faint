#include "hzpch.h"
#include "Entity.h"
#include "SceneSerializer.h"

#include "Renderer/SceneRenderer.h"
#include "Scripting/ScriptingEngineNet.h"
#include "Threading/JobSystem.h"
#include "FileSystem/File.h"

#include "Renderer/ShadersManager.h"

#include "AssetManagment/Project.h"

#include "Components.h"
#include "Engine.h"

#include "UI/Font/Font.h"
#include "UI/Font/FontLoader.h"

#include "Components/ParentComponent.h"
#include "Components/PrefabComponent.h"
#include "Systems/UISystem.h"
#include "Systems/AudioSystem.h"

namespace Faint {

	std::vector<CompilationError> errors;

	Scene::Scene() {

		if (environment == nullptr)
			environment = CreateRef<Environment>();

		m_systems = std::vector<Ref<System>>();
		m_EditorCamera = CreateRef<EditorCamera>(30.0f, 1.7798f, 0.1f, 500.0f);

		m_ScriptingSystem = CreateRef<ScriptingSystem>(this);

		// Adding systems - Order is important
		m_systems.push_back(CreateRef<PhysicsSystem>(this));
		m_systems.push_back(CreateRef<UISystem>(this));
		m_systems.push_back(m_ScriptingSystem);
		m_systems.push_back(CreateRef<TransformSystem>(this));
		m_systems.push_back(CreateRef<AudioSystem>(this));
	}

	Scene::~Scene() {

	}

	bool Scene::OnInit()
	{
		preInitializeDelegate.Broadcast();

		for (auto& system : m_systems) {
			if (!system->Init()) {
				HZ_CORE_WARN("Something don't want to Initialize!!");
				return false;
			}
		}

		postInitializeDelegate.Broadcast();

		return true;
	}

	void Scene::OnExit()
	{
		for (auto& system : m_systems) {
			system->Exit();
		}
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<NameComponent>(srcEntity).id);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src) {
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src) {
		CopyComponentIfExists<Component...>(dst, src);
	}

	void Scene::Update(Time time)
	{
		if (!Engine::IsPlayMode()) {
			
			// ...
		}

		for (auto& system : m_systems)
			system->Update(time);

		SceneRenderer::Update(time);
	}

	void Scene::EditorUpdate(Time time)
	{
		m_EditorCamera->OnUpdate(time);
	}

	void Scene::FixedUpdate(Time time)
	{
		for (auto& system : m_systems)
			system->FixedUpdate(time);
	}

	void Scene::Draw(Framebuffer2& framebuffer)
	{
		Ref<Camera> cam = nullptr;
		const auto& view = _registry.view<TransformComponent, CameraComponent>();
		for (const auto& e : view)
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
			cam = camera.camera;

			cam->SetPosition(transform.GetGlobalPosition());
		}
		if (!cam)
			return;

		SceneRenderer::BeginRenderScene(cam->GetProjectionMatrix(), cam->GetViewMatrix(), cam->GetPosition());
		SceneRenderer::RenderScene(*this, framebuffer);
	}

	void Scene::Draw(Framebuffer2& framebuffer, const Matrix4& projection, const Matrix4& view)
	{
		SceneRenderer::BeginRenderScene(m_EditorCamera->GetProjectionMatrix(), m_EditorCamera->GetViewMatrix(), m_EditorCamera->GetPosition());
		SceneRenderer::RenderScene(*this, framebuffer);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->_viewportWidth = other->_viewportWidth;
		newScene->_viewportHeight = other->_viewportHeight;

		auto& srcSceneRegistry = other->_registry;
		auto& dstSceneRegistry = newScene->_registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<NameComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<NameComponent>(e).id;
			const auto& name = srcSceneRegistry.get<NameComponent>(e).name;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except NameComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Ref<Scene> Scene::Copy(const std::string& path)
	{
		Ref<Scene> sceneCopy = Scene::New();

		SceneSerializer serializer(sceneCopy);
		if (serializer.Deserialize(path)) {
			return sceneCopy;
		}

		return Ref<Scene>();
	}

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> sceneCopy = Scene::New();

		//SceneSerializer serializer(sceneCopy);
		//if (serializer.Deserialize(FullPath)) {
		//	return sceneCopy;
		//}

		return Ref<Scene>();
	}

	Ref<Scene> Scene::New()
	{
		return CreateRef<Scene>();
	}

	Ref<Scene> Scene::Save()
	{
		return Ref<Scene>();
	}

	Entity Scene::CreateEntity(std::string name) {
		return CreateEntityWithUUID(UUID(), name);
	}
	Entity Scene::CreateEntityWithUUID(UUID uuid, std::string name) {

		if (name.empty()) {
			HZ_CORE_ERROR("Failed to create entity. Entity name caanot be empty.");
			return Entity();
		}

		std::string entityName;
		if (!EntityExists(name)) {
			entityName = name;
		}
		
		// Try to generate a unique name
		for (uint32_t i = 1; i < 4096; i++) {
			const std::string& entityEnumName = name + std::to_string(i);
			const auto& entityId = GetEntity(entityEnumName).GetHandle();
			if (entityId == -1) {
				entityName = entityEnumName;
				break;
			}
		}

		Entity entity = { _registry.create(), this };

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<VisibilityComponent>();
		entity.AddComponent<ParentComponent>();
		entity.AddComponent<NameComponent>(name.empty() ? "Entity" : name);
		entity.GetComponent<NameComponent>().id = uuid;

		m_EntityIDMap[uuid] = entity;
		if (!name.empty())
			m_EntityNameMap[name] = entity;

		return entity;
	}
	void Scene::DestroyEntity(Entity entity) {

		ParentComponent& parentC = entity.GetComponent<ParentComponent>();
		std::vector<Entity> copyChildrens = parentC.Children;

		if (parentC.HasParent) {
			ParentComponent& parent = parentC.Parent.GetComponent<ParentComponent>();
			parent.RemoveChildren(entity);
		}

		for (auto & c : copyChildrens)
		{
			DestroyEntity(c);
		}

		// Remove from ID to Entity cache
		if (m_EntityIDMap.find(entity.GetComponent<NameComponent>().id) != m_EntityIDMap.end()) {
			m_EntityIDMap.erase(entity.GetComponent<NameComponent>().id);
		}

		if (m_EntityNameMap.find(entity.GetComponent<NameComponent>().name) != m_EntityNameMap.end()) {
			m_EntityNameMap.erase(entity.GetComponent<NameComponent>().name);
		}
		
		entity.Destroy();
	}

	bool Scene::EntityExists(const std::string& name) {
		return GetEntity(name).GetHandle() != -1;
	}

	std::vector<Entity> Scene::GetAllEntities() {
		std::vector<Entity> allEntities;
		auto view = _registry.view<NameComponent>();
		for (auto& e : view) {
			Entity newEntity(e, this);

			// Check if valid for deleted entities.
			if (newEntity.IsValid()) {
				allEntities.push_back(newEntity);
			}
		}

		// Temporary fix to prevent order of tree to change randomly until actual order is implemented.
		std::sort(allEntities.begin(), allEntities.end(), [](Entity a, Entity b) {
			return a.GetComponent<NameComponent>().name < b.GetComponent<NameComponent>().name;
		});
		return allEntities;
	}

	Entity Scene::GetEntity(const std::string& name) {
		if (m_EntityNameMap.find(name) != m_EntityNameMap.end()) {
			return m_EntityNameMap[name];
		}
		return Entity();
	}

	Entity Scene::GetEntityByUUID(UUID uuid) {
		// TODO(Yan): Maybe should be assert
		if (m_EntityIDMap.find(uuid) != m_EntityIDMap.end())
			return { m_EntityIDMap.at(uuid), this };
		return {};
	}

	Entity Scene::GetEntityByID(int id) {

		if (m_EntityIDMap.find(id) != m_EntityIDMap.end()) {
			return m_EntityIDMap[id];
		}

		auto idView = _registry.view<NameComponent>();
		for (auto e : idView) {
			NameComponent& nameC = idView.get<NameComponent>(e);
			if (nameC.id == id) {
				auto newEntity = Entity{ e, this };
				m_EntityIDMap[id] = newEntity;
				return newEntity;
			}
		}
		return Entity{ (entt::entity)0, this };
	}

	bool Scene::EntityIsParent(Entity entity, Entity parent) {

		if (!entity.IsValid())
			return false;

		if (entity.GetComponent<ParentComponent>().HasParent && entity.GetComponent<ParentComponent>().Parent == parent) {
			return true;
		}

		Entity current = entity;
		while (current.GetComponent<ParentComponent>().HasParent && current != parent) {
			current = current.GetComponent<ParentComponent>().Parent;

			if (current = parent) return true;
		}

		return false;
	}

	void Scene::OnViewportResize(float width, float height) {
		_viewportWidth = width;
		_viewportHeight = height;

		auto view = _registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.fixedAspectRatio) {
				//camera.camera->SetViewportSizee(width, height);
				camera.camera->SetProjectionMatrix(glm::perspective(glm::radians(camera.camera->m_fieldOfView), Window::Get()->viewportWidth / Window::Get()->viewportHeight, camera.camera->m_nearPlane, camera.camera->m_farPlane));
			}
		}
	}

	Entity Scene::DuplicateEntity(Entity entity) {
		
		std::string name = entity.GetComponent<NameComponent>().name;
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
		CopyComponentIfExists<PrefabComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<RigidBodyComponent>(newEntity, entity);
		CopyComponentIfExists<BoxColliderComponent>(newEntity, entity);
		CopyComponentIfExists<SphereColliderComponent>(newEntity, entity);
		CopyComponentIfExists<NetScriptComponent>(newEntity, entity);
		CopyComponentIfExists<WrenScriptComponent>(newEntity, entity);
		CopyComponentIfExists<LightComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteComponent>(newEntity, entity);
		CopyComponentIfExists<AudioComponent>(newEntity, entity);
		CopyComponentIfExists<TextComponent>(newEntity, entity);
		CopyComponentIfExists<TextBlitterComponent>(newEntity, entity);

		return newEntity;
	}

	Ref<Camera> Scene::GetCurrentCamera()
	{
		if (Engine::IsPlayMode())
		{
			Ref<Camera> cam = nullptr;
			{
				auto view = _registry.view<TransformComponent, CameraComponent>();
				for (auto e : view)
				{
					auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
					cam = camera.camera;
					break;
				}
			}

			if (!cam)
				cam = m_EditorCamera;

			return cam;
		}

		return m_EditorCamera;
	}

	Entity Scene::GetPrimaryCameraEntity() {
		auto view = _registry.view<CameraComponent>();
		for (auto entity : view) {
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.primary) {
				return Entity(entity, this);
			}
		}
		return {};
	}

	json Scene::Serialize()
	{
		BEGIN_SERIALIZE();
		
		END_SERIALIZE();
	}

	bool Scene::Deserialize(const json& j)
	{
		return true;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component) {
		//static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {
		
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
		if (_viewportWidth > 0 && _viewportHeight > 0) {
			component.camera = CreateRef<Camera>();
			//component.camera->SetViewportSizee(_viewportWidth, _viewportHeight);
			component.camera->SetProjectionMatrix(glm::perspective(glm::radians(component.camera->m_fieldOfView), Window::Get()->viewportWidth / Window::Get()->viewportHeight, component.camera->m_nearPlane, component.camera->m_farPlane));
		}
	}

	template<>
	void Scene::OnComponentAdded<NameComponent>(Entity entity, NameComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CharacterControllerComponent>(Entity entity, CharacterControllerComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PrefabComponent>(Entity entity, PrefabComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NetScriptComponent>(Entity entity, NetScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<LuaScriptComponent>(Entity entity, LuaScriptComponent& component) {
		if (component.path == "") {
			component.path = FileSystem::Root + "Scripts/player.lua";

		}
	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<AudioComponent>(Entity entity, AudioComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<WrenScriptComponent>(Entity entity, WrenScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TextComponent>(Entity entity, TextComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TextBlitterComponent>(Entity entity, TextBlitterComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BSPBrushComponent>(Entity entity, BSPBrushComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyComponent>(Entity entity, SkyComponent& component)
	{
	}
}
