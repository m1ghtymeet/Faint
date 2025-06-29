#include "hzpch.h"
#include "Entity.h"
#include "Scene.h"

#include <algorithm>
#include "Components.h"

Faint::Event<Faint::Entity&> Faint::Entity::CreatedEvent;

Faint::Entity::Entity(int64_t p_gbID, const std::string& p_name, const std::string& p_tag, bool& p_playing) :
	m_id(p_gbID),
	m_name(p_name),
	m_tag(p_tag),
	m_playing(p_playing)
	//transform(AddComponent<TransformComponent>())
{
	
	CreatedEvent.Invoke(*this);
}

Faint::Entity::Entity(entt::entity handle, Scene* scene, bool& p_playing) :
	m_playing(p_playing)
	//transform(AddComponent<TransformComponent>())
{
}

Faint::Entity::~Entity() {

	std::for_each(m_components.begin(), m_components.end(), [&](std::shared_ptr<AComponent> p_component) { ComponentRemovedEvent.Invoke(*p_component); });
}

const std::string& Faint::Entity::GetName() const {
	return m_name;
}

void Faint::Entity::SetName(const std::string& p_name) {
	m_name = p_name;
}

void Faint::Entity::SetActive(bool p_active) {
	if (p_active != m_active) {
		m_active = p_active;
	}
}

bool Faint::Entity::IsActive() const {
	return m_active;
}

void Faint::Entity::SetID(int64_t p_id) {
	m_id = p_id;
}

int64_t Faint::Entity::GetID() const {
	return m_id;
}

void Faint::Entity::SetParent(Faint::Entity& p_parent) {
	
	/* Define the given parent as the new parent */
	m_parent = &p_parent;
	m_parentID = p_parent.m_id;
	//tranform.setParent
	
	/* Store the gameobject in the parent children list */
	p_parent.m_children.push_back(this);
}

bool Faint::Entity::HasParent() const {
	return m_parent;
}

Faint::Entity* Faint::Entity::GetParent() const {
	return m_parent;
}

std::vector<Faint::Entity*>& Faint::Entity::GetChildren() {
	return m_children;
}

void Faint::Entity::Destroy() {
	
}

void Faint::Entity::SetSleeping(bool p_sleeping) {
	m_sleeping = p_sleeping;
}

void Faint::Entity::OnAwake() {
	m_awaked = true;
	std::for_each(m_components.begin(), m_components.end(), [](auto element) {element->OnAwake(); });
}

json Faint::Entity::Serialize() {
	BEGIN_SERIALIZE();
	END_SERIALIZE();
}

void Faint::Entity::Deserialize(const json& entity) {
	std::string name = entity["NameComponent"]["Name"];
	uint64_t uuid = (uint64_t)entity["NameComponent"]["ID"];
	HZ_CORE_TRACE("Deserialized entity with ID = " + std::to_string(uuid) + ", name = " + name + "");
	
	if (entity.contains("Transform")) {
		json transformC = entity["Transform"];
		//deserializedEntity->transform.SetLocalPosition(glm::vec3(transformC["LocalPosition"]["x"], transformC["LocalPosition"]["y"], transformC["LocalPosition"]["z"]));
		//deserializedEntity->transform.SetLocalRotation(glm::quat(glm::vec3(transformC["LocalRotation"]["x"], transformC["LocalRotation"]["y"], transformC["LocalRotation"]["z"])));
		//deserializedEntity->transform.SetLocalScale(glm::vec3(transformC["LocalScale"]["x"], transformC["LocalScale"]["y"], transformC["LocalScale"]["z"]));
	}
	
	if (entity.contains("Camera")) {
		json cameraComponent = entity["Camera"];
		CameraComponent cc = AddComponent<CameraComponent>();
		cc.camera = CreateRef<Camera>();
		cc.camera->m_fieldOfView = cameraComponent["FOV"];
		cc.camera->m_nearPlane = cameraComponent["NearPlane"];
		cc.camera->m_farPlane = cameraComponent["FarPlane"];
		cc.primary = cameraComponent["Primary"];
		cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"];
	}
	
	if (entity.contains("MeshRenderer")) {
		json meshRendererComponent = entity["MeshRenderer"];
		MeshRendererComponent meshRenderer = AddComponent<MeshRendererComponent>();
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
		LightComponent& light = AddComponent<LightComponent>();
		light.Radius = lightComponent["Radius"];
		light.Strength = lightComponent["Strength"];
		DESERIALIZE_VEC3(lightComponent["Color"], light.Color);
		light.Type = (LightType)lightComponent["Type"];
	}
	
	if (entity.contains("NetScript")) {
		json netScriptComponent = entity["NetScript"];
		NetScriptComponent& net = AddComponent<NetScriptComponent>();
		net.ScriptPath = netScriptComponent["Path"];
	}
}