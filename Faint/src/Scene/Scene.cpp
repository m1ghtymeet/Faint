
#include <algorithm>
#include <string>

#include "Entity.h"

#include "Renderer/SceneRenderer.h"
#include "Scripting/ScriptingEngineNet.h"
#include "Threading/JobSystem.h"
#include "FileSystem/File.h"

#include "AssetManagment/Project.h"

#include "Components.h"
#include "Engine.h"

#include "Components/PrefabComponent.h"
#include "Systems/UISystem.h"
#include "Systems/AudioSystem.h"

namespace Faint {

	std::vector<CompilationError> errors;

	Scene::Scene() {
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

	void Scene::Play() {
		m_isPlaying = true;

		/* Wake up gameobjects to allow them to react to OnEnable, OnDisable and OnDestroy */
		std::for_each(m_entities.begin(), m_entities.end(), [](Entity* p_element) {  });

		std::for_each(m_entities.begin(), m_entities.end(), [](Entity* p_element) { if (p_element->IsActive()) p_element->OnAwake(); });
		//std::for_each(m_entities.begin(), m_entities.end(), [](Entity* p_element) { if (p_element->IsActive()) p_element->OnEnable(); });
		//std::for_each(m_entities.begin(), m_entities.end(), [](Entity* p_element) { if (p_element->IsActive()) p_element->OnStart(); });
	}

	bool Scene::OnInit() {

		for (auto& system : m_systems) {
			if (!system->Init()) {
				HZ_CORE_WARN("Something don't want to Initialize!!");
				return false;
			}
		}

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
				//auto view = src.view<Component>();
				for (auto srcEntity : Engine::GetCurrentScene()->GetAllEntities()) {
					entt::entity dstEntity = enttMap.at(srcEntity.GetID());

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
		CopyComponent<Component...>(dst, src, enttMap);
	}

	//template<typename... Component>
	//static void CopyComponentIfExists(Entity dst, Entity src) {
	//	([&]()
	//		{
	//			if (src.HasComponent<Component>())
	//				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	//		}(), ...);
	//}

	//template<typename... Component>
	//static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src) {
	//	CopyComponentIfExists<Component...>(dst, src);
	//}

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

	void Scene::Draw() {
		Ref<Camera> cam = nullptr;
		const auto& view = _registry.view<TransformComponent, CameraComponent>();
		for (const auto& e : view)
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
			cam = camera.camera;

			cam->SetPosition(transform.GetGlobalPosition());
		}
		//if (!cam)
		//	return;

		SceneRenderer::BeginRenderScene(cam->GetProjectionMatrix(), cam->GetViewMatrix(), cam->GetPosition());
		SceneRenderer::RenderScene(*this);
	}

	void Scene::Draw(const Matrix4& projection, const Matrix4& view) {
		SceneRenderer::BeginRenderScene(m_EditorCamera->GetProjectionMatrix(), m_EditorCamera->GetViewMatrix(), m_EditorCamera->GetPosition());
		SceneRenderer::RenderScene(*this);
	}

	Ref<Scene> Scene::Save() {
		

		return Ref<Scene>();
	}

	std::string Scene::GetUniqueEntityName(const std::string& name) {
		std::string entityName;
		if (!EntityExists(name)) {
			return name;
		}

		// Try to generate a unique name
		for (uint32_t i = 1; i < 4096; i++) {
			const std::string& entityEnumName = name + " (" + std::to_string(i) + ")";
			Entity* entityId = GetEntityByName(entityEnumName);
			if (!entityId) {
				return entityEnumName;
			}
		}

		// We ran out of names
		HZ_CORE_WARN("Failed to create unique entity name. Limit reached with name: " + name);
		return name;
	}

	Entity& Scene::CreateEntity(std::string name) {
		return CreateEntity(name, "");
	}

	Entity& Scene::CreateEntity(const std::string& p_name, const std::string& p_tag) {
		std::string entityName = GetUniqueEntityName(p_name);

		m_entities.push_back(new Entity(m_availableID++, entityName, "", m_isPlaying));
		Entity& instance = *m_entities.back();
		instance.AddComponent<TransformComponent>();
		//instance.ComponentAddedEvent += std::bind(&)
		if (m_isPlaying) {
			if (instance.IsActive()) {
				instance.SetSleeping(false);
				instance.OnAwake();
			}
		}

		m_EntityIDMap[instance.GetID()] = &instance;
		m_EntityNameMap[entityName] = &instance;

		return instance;
	}

	void Scene::DestroyEntity(Entity& p_entity) {


		// Remove from ID to Entity cache
		if (m_EntityIDMap.find(p_entity.GetID()) != m_EntityIDMap.end()) {
			m_EntityIDMap.erase(p_entity.GetID());
		}
		if (m_EntityNameMap.find(p_entity.GetName()) != m_EntityNameMap.end()) {
			m_EntityNameMap.erase(p_entity.GetName());
		}
		
		p_entity.Destroy();
	}

	bool Scene::EntityExists(const std::string& name) {
		return m_EntityNameMap.find(name) != m_EntityNameMap.end();
	}

	std::vector<Entity*>& Scene::GetAllEntities() {
		return m_entities;
	}

	Entity* Scene::GetEntityByName(const std::string& p_name) const {
		auto it = m_EntityNameMap.find(p_name);
		if (it != m_EntityNameMap.end())
			return it->second;
		return nullptr;
	}

	Entity* Scene::GetEntityByTag(const std::string& p_tag) const {
		//auto result = std::find_if(m_entities.begin(), m_entities.end(), [tag](Entity* e) {
		//	return e->GetTag() == tag;
		//	});
		//if (result != m_entities.end())
		//	return *result;
		return nullptr;
	}

	Entity* Scene::GetEntityByID(int64_t p_id) const {
		auto it = m_EntityIDMap.find(p_id);
		if (it != m_EntityIDMap.end())
			return it->second;
		return nullptr;
	}

	//Entity Scene::GetEntity(const std::string& name) {
	//	//if (m_EntityNameMap.find(name) != m_EntityNameMap.end()) {
	//	//	return m_EntityNameMap[name];
	//	//}
	//	//return Entity();
	//}

	void Scene::OnViewportResize(float width, float height) {
		_viewportWidth = width;
		_viewportHeight = height;

		auto view = _registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.fixedAspectRatio) {
				camera.camera->SetProjectionMatrix(glm::perspective(glm::radians(camera.camera->m_fieldOfView), Window::Get()->viewportWidth / Window::Get()->viewportHeight, camera.camera->m_nearPlane, camera.camera->m_farPlane));
			}
		}
	}

	Entity Scene::DuplicateEntity(Entity entity) {
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		//CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
		//CopyComponentIfExists<PrefabComponent>(newEntity, entity);
		//CopyComponentIfExists<CameraComponent>(newEntity, entity);
		//CopyComponentIfExists<RigidBodyComponent>(newEntity, entity);
		//CopyComponentIfExists<BoxColliderComponent>(newEntity, entity);
		//CopyComponentIfExists<SphereColliderComponent>(newEntity, entity);
		//CopyComponentIfExists<NetScriptComponent>(newEntity, entity);
		//CopyComponentIfExists<WrenScriptComponent>(newEntity, entity);
		//CopyComponentIfExists<LightComponent>(newEntity, entity);
		//CopyComponentIfExists<SpriteComponent>(newEntity, entity);
		//CopyComponentIfExists<AudioComponent>(newEntity, entity);
		//CopyComponentIfExists<TextComponent>(newEntity, entity);
		//CopyComponentIfExists<TextBlitterComponent>(newEntity, entity);

		return newEntity;
	}

	Ref<Camera> Scene::GetCurrentCamera() {
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

	Entity* Scene::GetPrimaryCameraEntity() {
		//auto view = _registry.view<CameraComponent>();
		//for (auto entity : view) {
		//	const auto& camera = view.get<CameraComponent>(entity);
		//	if (camera.primary) {
		//		return Entity(entity, this);
		//	}
		//}
		//return;
		return nullptr;
	}

	const Scene::FastAccessComponents& Scene::GetFastAccessComponents() const {
		return m_fastAccessComponents;
	}

	json SerializeEntity(Entity* entity) {
		BEGIN_SERIALIZE();
		j["Transform"] = entity->GetComponent<TransformComponent>()->Serialize();
		if (entity->GetComponent<CameraComponent>()) j["Camera"] = entity->GetComponent<CameraComponent>()->Serialize();
		if (entity->GetComponent<MeshRendererComponent>()) j["MeshRenderer"] = entity->GetComponent<MeshRendererComponent>()->Serialize();
		if (entity->GetComponent<LightComponent>()) j["Light"] = entity->GetComponent<LightComponent>()->Serialize();
		if (entity->GetComponent<BoxColliderComponent>()) j["BoxCollider"] = entity->GetComponent<BoxColliderComponent>()->Serialize();
		if (entity->GetComponent<SphereColliderComponent>()) j["SphereCollider"] = entity->GetComponent<SphereColliderComponent>()->Serialize();
		if (entity->GetComponent<RigidBodyComponent>()) j["RigidBody"] = entity->GetComponent<RigidBodyComponent>()->Serialize();
		if (entity->GetComponent<NetScriptComponent>()) j["NetScript"] = entity->GetComponent<NetScriptComponent>()->Serialize();
		if (entity->GetComponent<LuaScriptComponent>()) j["LuaScript"] = entity->GetComponent<LuaScriptComponent>()->Serialize();
		if (entity->GetComponent<AudioComponent>()) j["Audio"] = entity->GetComponent<AudioComponent>()->Serialize();
		END_SERIALIZE();
	}

	json Scene::Serialize() {
		BEGIN_SERIALIZE();
		j["Entities"] = json::object();
		for (Entity* entity : Engine::GetCurrentScene()->GetAllEntities()) {
			if (entity) continue;
			j["Entities"][entity->GetName()] = SerializeEntity(entity);
		}
		END_SERIALIZE();
	}

	void Scene::Deserialize(const json& j) {
		json entities = j["Entities"];
		if (j.contains("Entities")) {
			for (auto& [name, entity] : entities.items()) {
				int64_t maxID = 1;

				std::string name = entity["NameComponent"]["Name"];
				uint64_t uuid = (uint64_t)entity["NameComponent"]["ID"];
				HZ_CORE_TRACE("Deserialized entity with ID = " + std::to_string(uuid) + ", name = " + name + "");
				Entity& deserializedEntity = Engine::GetCurrentScene()->CreateEntity(name);

				if (entity.contains("Transform")) {
					json transformC = entity["Transform"];
					TransformComponent* transform = deserializedEntity.GetComponent<TransformComponent>();
					transform->SetLocalPosition(glm::vec3(transformC["LocalPosition"]["x"], transformC["LocalPosition"]["y"], transformC["LocalPosition"]["z"]));
					transform->SetLocalRotation(glm::quat(glm::vec3(transformC["LocalRotation"]["x"], transformC["LocalRotation"]["y"], transformC["LocalRotation"]["z"])));
					transform->SetLocalScale(glm::vec3(transformC["LocalScale"]["x"], transformC["LocalScale"]["y"], transformC["LocalScale"]["z"]));
				}

				if (entity.contains("Camera")) {
					json cameraComponent = entity["Camera"];
					CameraComponent& cc = deserializedEntity.AddComponent<CameraComponent>();
					cc.camera = CreateRef<Camera>();
					cc.camera->m_fieldOfView = cameraComponent["FOV"];
					cc.camera->m_nearPlane = cameraComponent["NearPlane"];
					cc.camera->m_farPlane = cameraComponent["FarPlane"];
					cc.primary = cameraComponent["Primary"];
					cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"];
				}

				if (entity.contains("MeshRenderer")) {
					json meshRendererComponent = entity["MeshRenderer"];
					MeshRendererComponent& meshRenderer = deserializedEntity.AddComponent<MeshRendererComponent>();
					std::string modelPath = meshRendererComponent["ModelPath"];
					Model* model = AssetManager::LoadModel(modelPath, true);
					meshRenderer.ModelPath = modelPath;
					meshRenderer.SetModel(model);

					for (uint32_t i = 0; i < std::size(meshRenderer.GetModel()->GetMeshes()); i++) {
						Ref<Mesh> mesh = meshRenderer.GetModel()->GetMeshes()[i];
						auto meshR = meshRendererComponent["Model"]["Meshes"];
						bool loadedMaterialFile = false;
						const std::string materialPath = meshR[i]["MaterialPath"];
						if (!materialPath.empty()) {
							Ref<Material> newMaterial = AssetManager::LoadMaterial(materialPath);
							mesh->SetMaterial(newMaterial);
							loadedMaterialFile = true;
						}
						if (!loadedMaterialFile) {
							Ref<Material> material = mesh->GetMaterial();
							material = CreateRef<Material>();
						}
					}
				}

				if (entity.contains("Light")) {
					json lightComponent = entity["Light"];
					LightComponent& light = deserializedEntity.AddComponent<LightComponent>();
					light.Radius = lightComponent["Radius"];
					light.Strength = lightComponent["Strength"];
					DESERIALIZE_VEC3(lightComponent["Color"], light.Color);
					light.Type = (LightType)lightComponent["Type"];
				}

				if (entity.contains("NetScript")) {
					json netScriptComponent = entity["NetScript"];
					NetScriptComponent& net = deserializedEntity.AddComponent<NetScriptComponent>();
					net.ScriptPath = netScriptComponent["Path"];
				}
			}
		}
		else {
			HZ_CORE_ERROR("Cannot Find Entities Tree!");
		}
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
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkinnedMeshRendererComponent>(Entity entity, SkinnedMeshRendererComponent& component)
	{
		component.Model = AssetManager::LoadSkinnedModel("D:/C++ Projects/Heavy/Heavy/assets/models/Neutral_F.fbx", true);
	}

	template<>
	void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component) {
		auto viewMesh = Reg().view<TransformComponent, BoxColliderComponent, MeshRendererComponent>();
		for (auto e : viewMesh) {
			auto [transform, box, meshC] = viewMesh.get<TransformComponent, BoxColliderComponent, MeshRendererComponent>(e);
			for (auto mesh : meshC.GetModel()->GetMeshes()) {
				if (mesh->GetAABB().boundsMin.x > 0.0f ||
					mesh->GetAABB().boundsMin.y > 0.0f || 
					mesh->GetAABB().boundsMin.z > 0.0f ||
					mesh->GetAABB().boundsMax.x > 0.0f || 
					mesh->GetAABB().boundsMax.y > 0.0f || 
					mesh->GetAABB().boundsMax.z > 0.0f) {
					glm::vec3 scale = transform.GetGlobalScale();
					glm::vec3 fullSize = mesh->GetAABB().boundsMax - mesh->GetAABB().boundsMin;
					box.halfExtents = (fullSize * scale) * 0.5f;
				}
			}
		}
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CharacterControllerComponent>(Entity entity, CharacterControllerComponent& component)
	{
		component.Create();
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
	void Scene::OnComponentAdded<AudioComponent>(Entity entity, AudioComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<WrenScriptComponent>(Entity entity, WrenScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyComponent>(Entity entity, SkyComponent& component)
	{
	}
}
