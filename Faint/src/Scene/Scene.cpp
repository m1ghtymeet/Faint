
#include <algorithm>
#include <string>

#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>

#include "AssetManagment/Project.h"

#include "Components.h"
#include <Core/Window.h>
#include "Scene.h"

Moon::Scene::Scene() {
}

Moon::Scene::~Scene() {
	//std::for_each(m_entities.begin(), m_entities.end(), [](Entity*& element) {
	//	delete element;
	//});
	m_entities.clear();
}

void Moon::Scene::AddDefaultCamera() {
	auto& camera = CreateEntity("Main Camera");
	camera.AddComponent<CameraComponent>();
	camera.transform->SetLocalPosition({ 0.0f, 3.0f, 8.0f });
	camera.transform->SetLocalRotation(glm::quat({ 20.0f, 180.0f, 0.0f }));
}

void Moon::Scene::AddDefaultLights() {
	auto& pointLight = CreateEntity("Point Light");
	pointLight.AddComponent<LightComponent>();
	pointLight.transform->SetLocalPosition({ 0.0f, 5.0f, 0.0f });
}

void Moon::Scene::Play() {

	m_isPlaying = true;

	/* Wake up gameobjects to allow them to react to OnEnable, OnDisable and OnDestroy */
	std::for_each(m_entities.begin(), m_entities.end(), [](const std::unique_ptr<Entity>& p_element) {  });

	std::for_each(m_entities.begin(), m_entities.end(), [](const std::unique_ptr<Entity>& p_element) { if (p_element->IsActive()) p_element->OnAwake(); });
	std::for_each(m_entities.begin(), m_entities.end(), [](const std::unique_ptr<Entity>& p_element) { if (p_element->IsActive()) p_element->OnEnable(); });
	std::for_each(m_entities.begin(), m_entities.end(), [](const std::unique_ptr<Entity>& p_element) { p_element->OnStart(); });
}

bool Moon::Scene::IsPlaying() const {
	return m_isPlaying;
}

void Moon::Scene::Update(float p_deltaTime) {
	// ZoneScoped

	auto& entities = m_entities;
	std::for_each(entities.begin(), entities.end(), std::bind(std::mem_fn(&Entity::OnUpdate), std::placeholders::_1, p_deltaTime));
}

void Moon::Scene::FixedUpdate(float p_deltaTime) {
	// ZoneScoped
	//auto entities = m_entities;
	//std::for_each(entities.begin(), entities.end(), std::bind(std::mem_fn(&Entity::), std::placeholders::_1, p_deltaTime));
}

std::string Moon::Scene::GetUniqueEntityName(const std::string& name) {
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

Moon::Entity& Moon::Scene::CreateEntity(std::string name) {
	return CreateEntity(name, "");
}

Moon::Entity& Moon::Scene::CreateEntity(const std::string& p_name, const std::string& p_tag) {
	std::string entityName = GetUniqueEntityName(p_name);

	m_entities.push_back(std::make_unique<Entity>(m_availableID++, entityName, p_tag, m_isPlaying));
	Entity& instance = *m_entities.back();
	instance.ComponentAddedEvent += std::bind(&Scene::OnComponentAdded, this, std::placeholders::_1);
	instance.ComponentRemovedEvent += std::bind(&Scene::OnComponentRemoved, this, std::placeholders::_1);
	if (m_isPlaying) {
		if (instance.IsActive()) {
			instance.SetSleeping(false);
			instance.OnAwake();
			instance.OnEnable();
			instance.OnStart();
		}
	}

	m_EntityIDMap[instance.GetID()] = &instance;
	m_EntityNameMap[entityName] = &instance;

	return instance;
}

bool Moon::Scene::DestroyEntity(Entity& p_entity) {
	auto found = std::find_if(
		m_entities.begin(),
		m_entities.end(),
		[&p_entity](const std::unique_ptr<Entity>& element) {
			return element.get() == &p_entity;
		}
	);
	if (found == m_entities.end())
		return false;

	auto& children = p_entity.GetChildren();
	while (!children.empty()) {
		Entity* child = children.back();
		DestroyEntity(*child);
	}

	/*if (Entity* parent = p_entity.GetParent()) {
		auto& parentChildren = parent->GetChildren();
		parentChildren.erase(
			std::remove_if(parentChildren.begin(), parentChildren.end(), [&p_entity](Entity* e) {
				return e == &p_entity;
			}), parentChildren.end()
		);
	}*/

	// Remove from ID to Entity cache
	m_EntityIDMap.erase(p_entity.GetID());
	m_EntityNameMap.erase(p_entity.GetName());

	m_entities.erase(found);

	return true;
}

void Moon::Scene::CollectGarbages() {
	m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
		[](const std::unique_ptr<Entity>& element) {
			return !element->IsActive();
		}),
		m_entities.end()
	);
}

bool Moon::Scene::EntityExists(const std::string& name) {
	return m_EntityNameMap.find(name) != m_EntityNameMap.end();
}

std::vector<std::unique_ptr<Moon::Entity>>& Moon::Scene::GetAllEntities() {
	return m_entities;
}

const std::vector<std::unique_ptr<Moon::Entity>>& Moon::Scene::GetAllEntities() const {
	return m_entities;
}

Moon::Entity* Moon::Scene::GetEntityByName(const std::string& p_name) const {
	auto it = m_EntityNameMap.find(p_name);
	if (it != m_EntityNameMap.end())
		return it->second;
	return nullptr;
}

Moon::Entity* Moon::Scene::GetEntityByTag(const std::string& p_tag) const {
	auto result = std::find_if(m_entities.begin(), m_entities.end(), [&p_tag](const std::unique_ptr<Entity>& e) {
		return e->GetTag() == p_tag;
	});
	if (result != m_entities.end())
		return result->get();
	else
		return nullptr;
}

Moon::Entity* Moon::Scene::GetEntityByID(int64_t p_id) const {
	auto it = m_EntityIDMap.find(p_id);
	if (it != m_EntityIDMap.end())
		return it->second;
	return nullptr;
}

Moon::CameraComponent* Moon::Scene::FindMainCamera() const {
	for (CameraComponent* camera : m_fastAccessComponents.cameras) {
		if (camera->owner.IsActive()) {
			return camera;
		}
	}
	return nullptr;
}

void Moon::Scene::OnComponentAdded(AComponent& p_component) {

	if (auto result = dynamic_cast<MeshRendererComponent*>(&p_component))
		m_fastAccessComponents.modelRenderers.push_back(result);
	if (auto result = dynamic_cast<SkinnedMeshRendererComp*>(&p_component))
		m_fastAccessComponents.skinnedModelRenderers.push_back(result);
	if (auto result = dynamic_cast<CameraComponent*>(&p_component))
		m_fastAccessComponents.cameras.push_back(result);
	if (auto result = dynamic_cast<LightComponent*>(&p_component))
		m_fastAccessComponents.lights.push_back(result);
	if (auto result = dynamic_cast<AudioComponent*>(&p_component))
		m_fastAccessComponents.audios.push_back(result);
	if (auto result = dynamic_cast<Text2DComponent*>(&p_component))
		m_fastAccessComponents.texts.push_back(result);
	if (auto result = dynamic_cast<UI::WidgetComponent*>(&p_component))
		m_fastAccessComponents.widgets.push_back(result);
}

void Moon::Scene::OnComponentRemoved(AComponent& p_component) {

	if (auto result = dynamic_cast<CameraComponent*>(&p_component))
		m_fastAccessComponents.cameras.erase(std::remove(m_fastAccessComponents.cameras.begin(), m_fastAccessComponents.cameras.end(), result), m_fastAccessComponents.cameras.end());
	if (auto result = dynamic_cast<MeshRendererComponent*>(&p_component))
		m_fastAccessComponents.modelRenderers.erase(std::remove(m_fastAccessComponents.modelRenderers.begin(), m_fastAccessComponents.modelRenderers.end(), result), m_fastAccessComponents.modelRenderers.end());
	if (auto result = dynamic_cast<SkinnedMeshRendererComp*>(&p_component))
		m_fastAccessComponents.skinnedModelRenderers.erase(std::remove(m_fastAccessComponents.skinnedModelRenderers.begin(), m_fastAccessComponents.skinnedModelRenderers.end(), result), m_fastAccessComponents.skinnedModelRenderers.end());
	if (auto result = dynamic_cast<LightComponent*>(&p_component))
		m_fastAccessComponents.lights.erase(std::remove(m_fastAccessComponents.lights.begin(), m_fastAccessComponents.lights.end(), result), m_fastAccessComponents.lights.end());
	if (auto result = dynamic_cast<AudioComponent*>(&p_component))
		m_fastAccessComponents.audios.erase(std::remove(m_fastAccessComponents.audios.begin(), m_fastAccessComponents.audios.end(), result), m_fastAccessComponents.audios.end());
	if (auto result = dynamic_cast<Text2DComponent*>(&p_component))
		m_fastAccessComponents.texts.erase(std::remove(m_fastAccessComponents.texts.begin(), m_fastAccessComponents.texts.end(), result), m_fastAccessComponents.texts.end());
	if (auto result = dynamic_cast<UI::WidgetComponent*>(&p_component))
		m_fastAccessComponents.widgets.erase(std::remove(m_fastAccessComponents.widgets.begin(), m_fastAccessComponents.widgets.end(), result), m_fastAccessComponents.widgets.end());
}

const Moon::Scene::FastAccessComponents& Moon::Scene::GetFastAccessComponents() const {
	return m_fastAccessComponents;
}

Moon::Entity& Moon::Scene::InstantiatePrefab(const std::string& prefabPath) {
	if (!FileSystem::FileExists(prefabPath, true)) {
		HZ_CORE_ERROR("Prefab not found: {}", prefabPath);
		return CreateEntity("Missing Prefab");
	}
	// Load Prefab File
	std::string contentFile = FileSystem::ReadFile(prefabPath, true);
	if (contentFile.empty()) {
		HZ_CORE_ERROR("Failed to read prefab file: {}", prefabPath);
		return CreateEntity("Empty Prefab");
	}
	json prefabJson;
	try {
		prefabJson = json::parse(contentFile);
	}
	catch (const json::parse_error& e) {
		HZ_CORE_ERROR("Failed to parse prefab JSON: {} | Error: {}", prefabPath, e.what());
		return CreateEntity("Invalid Prefab");
	}
	// Create new entity from prefab data
	std::string entityName = prefabJson.contains("Name") ? prefabJson["Name"].get<std::string>() + " (Instance)" : "Prefab Entity";
	Moon::Entity& instance = CreateEntity(entityName);

	instance.Deserialize(prefabJson);
	auto& prefab = instance.AddComponent<PrefabComponent>();
	prefab.filepath = prefabPath;
	prefab.overridenComponents = prefabJson;

	if (prefabJson.contains("Children") && prefabJson["Children"].is_array()) {
		for (const auto& childJson : prefabJson["Children"]) {
			InstantiatePrefabChild(instance, childJson);
		}
	}
	return instance;
}

Moon::Entity& Moon::Scene::InstantiatePrefabChild(Moon::Entity& parent, const json& childJson) {
	// Create new entity from prefab data
	std::string entityName = childJson.contains("Name") ? childJson["Name"].get<std::string>() + " (Instance)" : "Prefab Entity";
	Moon::Entity& instance = CreateEntity(entityName);
	instance.SetParent(parent);
	instance.transform->SetParent(*parent.transform);
	instance.Deserialize(childJson);
	if (childJson.contains("Children") && childJson["Children"].is_array()) {
		for (const auto& childJson : childJson["Children"]) {
			InstantiatePrefabChild(instance, childJson);
		}
	}
	return instance;
}

json SerializeEntity(Moon::Entity* entity) {
	return entity->Serialize();
}

json Moon::Scene::Serialize() {
	BEGIN_SERIALIZE();
	j["Entities"] = json::object();
	for (auto& entity : GetAllEntities()) {
		j["Entities"][entity->GetName()] = SerializeEntity(entity.get());
	}
	END_SERIALIZE();
}

void Moon::Scene::Deserialize(const json& j) {
	if (!j.contains("Entities")) {
		HZ_CORE_ERROR("Cannot Find Entities Tree!");
		return;
	}

	std::unordered_map<uint64_t, Entity*> entityMap;
	json entities = j["Entities"];
	for (auto& [name, entityJson] : entities.items()) {
		std::string entityName = name;
		std::string entityTag = entityJson["Tag"];
		Entity& entity = CreateEntity(entityName, entityTag);
		entity.Deserialize(entityJson);

		uint64_t id = entityJson["ID"];
		//entity.SetID(id);
		entityMap[id] = &entity;
	}

	for (auto& [name, entityJson] : entities.items()) {
		uint64_t id = entityJson["ID"];
		Entity* entity = entityMap[id];
		
		if (entityJson.contains("ParentID")) {
			uint64_t parentID = entityJson["ParentID"];
			if (entityMap.contains(parentID))
				entity->SetParent(*entityMap[parentID]);
		}
	}
}