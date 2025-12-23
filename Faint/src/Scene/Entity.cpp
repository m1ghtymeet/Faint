#include "Entity.h"
#include "Scene.h"
#include "Components.h"
#include <Debug/Log.h>

/* Message Box */ #include <Util/PlatformUtil.h>

#include <sol/sol.hpp>
#include <AssetManagment/MeshManager.h>
#include <AssetManagment/ModelManager.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <FileSystem/FileSystem.h>
#include <Physics/CharacterController.h>

Moon::Event<Moon::Entity&> Moon::Entity::CreatedEvent;
Moon::Event<Moon::Entity&> Moon::Entity::DestroyedEvent;

Moon::Entity::Entity(int64_t p_gbID, const std::string& p_name, const std::string& p_tag, bool& p_playing) :
	m_id(p_gbID),
	m_name(p_name),
	m_tag(p_tag),
	m_playing(p_playing)
{
	transform = &AddComponent<TransformComponent>();
	CreatedEvent.Invoke(*this);
}

Moon::Entity::~Entity() {

	if (!m_sleeping) {
		if (IsActive())
			OnDisable();

		if (m_awaked && m_started)
			OnDestroy();
	}

	DestroyedEvent.Invoke(*this);

	DetachFromParent();
	
	for (auto* child : m_children) {
		child->m_parent = nullptr;
		child->m_parentID = 0;
		//delete child;
	}
	m_children.clear();

	std::for_each(m_components.begin(), m_components.end(), [&](std::shared_ptr<AComponent> p_component) { ComponentRemovedEvent.Invoke(*p_component); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& p_behaviour) {  });
	//std::for_each(m_children.begin(), m_children.end(),		[&](Entity* p_element) { delete p_element; });
}

const std::string& Moon::Entity::GetName() const {
	return m_name;
}

const std::string& Moon::Entity::GetTag() const {
	return m_tag;
}

void Moon::Entity::SetName(const std::string& p_name) {
	m_name = p_name;
}

void Moon::Entity::SetTag(const std::string& p_tag) {
	m_tag = p_tag;
}

void Moon::Entity::SetActive(bool p_active) {
	if (p_active != m_active) {
		RecursiveWasActiveUpdate();
		m_active = p_active;
		RecursiveActiveUpdate();
	}
}

bool Moon::Entity::IsActive() const {
	return m_active;
}

void Moon::Entity::SetID(int64_t p_id) {
	m_id = p_id;
}

int64_t Moon::Entity::GetID() const {
	return m_id;
}

void Moon::Entity::SetParent(Moon::Entity& p_parent) {
	DetachFromParent();

	/* Define the given parent as the new parent */
	m_parent = &p_parent;
	m_parentID = p_parent.m_id;
	transform->SetParent(*p_parent.transform);
	
	/* Store the gameobject in the parent children list */
	if (m_parent)
		m_parent->AddChild(this);
}

void Moon::Entity::DetachFromParent() {
	if (m_parent) {
		m_parent->m_children.erase(std::remove_if(m_parent->m_children.begin(), m_parent->m_children.end(), [this](Entity* p_element)
			{ return p_element == this; }
		));
	}

	m_parent = nullptr;
	m_parentID = 0;

	transform->RemoveParent();
}

bool Moon::Entity::HasParent() const {
	return m_parent;
}

Moon::Entity* Moon::Entity::GetParent() const {
	return m_parent;
}

Moon::Entity* Moon::Entity::GetChildByName(const std::string& name) const {
	for (Entity* child : m_children) {
		if (child && child->GetName() == name)
			return child;
	}
	HZ_CORE_ERROR("Entity::GetChildByName() Cannot find child by name: {0}", name);
	return nullptr;
}

void Moon::Entity::AddChild(Entity* child) {
	if (!child || child == this) return;

	if (child->m_parent)
		child->m_parent->RemoveChild(child);

	m_children.push_back(child);
	child->m_parent = this;
}

void Moon::Entity::RemoveChild(Entity* child) {
	if (!child) return;

	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end()) {
		m_children.erase(it);
		child->m_parent = nullptr;
	}
}

std::vector<Moon::Entity*>& Moon::Entity::GetChildren() {
	return m_children;
}

void Moon::Entity::Destroy() {
	m_destroyed = true;

	for (auto child : m_children)
		child->Destroy();
}

void Moon::Entity::SetSleeping(bool p_sleeping) {
	m_sleeping = p_sleeping;
}

void Moon::Entity::OnAwake() {
	m_awaked = true;
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnAwake(); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [](auto& element) { element.second.OnAwake(); });
}

void Moon::Entity::OnStart() {

	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnStart(); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [](auto& element) { element.second.OnStart(); });
}

void Moon::Entity::OnEnable() {

	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnEnable(); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [](auto& element) { element.second.OnEnable(); });
}

void Moon::Entity::OnDisable() {

	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnDisable(); });
}

void Moon::Entity::OnDestroy() {
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnDestroy(); });
}

void Moon::Entity::OnUpdate(float p_deltaTime) {
	if (IsActive()) {
		std::for_each(m_components.begin(), m_components.end(), [&](auto element) {element->OnUpdate(p_deltaTime); });
		std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnUpdate(p_deltaTime); });
	}
}

void Moon::Entity::OnCollisionEnter(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnCollisionEnter(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnCollisionEnter(otherBody); });
}

void Moon::Entity::OnCollisionStay(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnCollisionStay(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnCollisionStay(otherBody); });
}

void Moon::Entity::OnCollisionExit(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnCollisionExit(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnCollisionExit(otherBody); });
}

void Moon::Entity::OnTriggerEnter(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnTriggerEnter(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnTriggerEnter(otherBody); });
}

void Moon::Entity::OnTriggerStay(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnTriggerStay(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnTriggerStay(otherBody); });
}

void Moon::Entity::OnTriggerExit(Moon::ColliderComponent& otherBody) {
	std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnTriggerExit(otherBody); });
	std::for_each(m_behaviours.begin(), m_behaviours.end(), [&](auto& element) { element.second.OnTriggerExit(otherBody); });
}

//std::vector<Ref<Moon::AComponent>>& Moon::Entity::GetComponents() {
//	return m_components;
//}

Moon::Behaviour& Moon::Entity::AddBehaviour(const std::string& p_name, const std::string& p_path) {
	
	m_behaviours.try_emplace(p_name, *this, p_name, p_path);
	Behaviour& newInstance = m_behaviours.at(p_name);
	if (m_playing && IsActive()) {
		newInstance.OnAwake();
		newInstance.OnEnable();
		newInstance.OnStart();
	}
	return newInstance;
}

bool Moon::Entity::RemoveBehaviour(Behaviour& p_behaviour) {

	bool found = false;

	for (auto& [name, behaviour] : m_behaviours) {
		if (&behaviour == &p_behaviour) {
			found = true;
			break;
		}
	}

	if (found)
		return RemoveBehaviour(p_behaviour.name);
	else
		return false;
}

bool Moon::Entity::RemoveBehaviour(const std::string& p_name) {

	Behaviour* found = GetBehaviour(p_name);
	if (found) {
		return m_behaviours.erase(p_name);
	}
	else
		return false;
}

Moon::Behaviour* Moon::Entity::GetBehaviour(const std::string& p_name) {
	auto it = m_behaviours.find(p_name);
	if (it != m_behaviours.end())
		return &it->second;
	return nullptr;
}

std::unordered_map<std::string, Moon::Behaviour>& Moon::Entity::GetBehaviours() {
	return m_behaviours;
}

json Moon::Entity::Serialize() {
	BEGIN_SERIALIZE();
	j["ID"] = (int)m_id;
	j["Tag"] = m_tag;
	if (HasParent())
		j["ParentID"] = m_parentID;

	if (GetComponent<Moon::PrefabComponent>())
		j["Prefab"] = GetComponent<Moon::PrefabComponent>()->Serialize();

	j["Transform"] = GetComponent<Moon::TransformComponent>()->Serialize();
	if (GetComponent<Moon::CameraComponent>()) j["Camera"] = GetComponent<Moon::CameraComponent>()->Serialize();
	if (GetComponent<Moon::MaterialRendererComp>()) j["MaterialRenderer"] = GetComponent<Moon::MaterialRendererComp>()->Serialize();
	if (GetComponent<Moon::MeshRendererComponent>()) j["MeshRenderer"] = GetComponent<Moon::MeshRendererComponent>()->Serialize();
	if (GetComponent<Moon::PointLightComponent>()) j["PointLight"] = GetComponent<Moon::PointLightComponent>()->Serialize();
	if (GetComponent<Moon::DirectionalLightComp>()) j["DirectionalLight"] = GetComponent<Moon::DirectionalLightComp>()->Serialize();
	if (GetComponent<Moon::RigidBodyComponent>()) j["RigidBody"] = GetComponent<Moon::RigidBodyComponent>()->Serialize();
	if (GetComponent<Moon::BoxColliderComponent>()) j["BoxCollider"] = GetComponent<Moon::BoxColliderComponent>()->Serialize();
	if (GetComponent<Moon::NetScriptComponent>()) j["NetScript"] = GetComponent<Moon::NetScriptComponent>()->Serialize();
	if (GetComponent<Moon::AudioComponent>()) j["AudioSource"] = GetComponent<Moon::AudioComponent>()->Serialize();
	if (GetComponent<Moon::AudioListenerComponent>()) j["AudioListener"] = GetComponent<Moon::AudioListenerComponent>()->Serialize();
	if (GetComponent<Moon::CharacterControllerComponent>()) j["CharacterController"] = GetComponent<Moon::CharacterControllerComponent>()->Serialize();
	if (GetComponent<Moon::Text2DComponent>()) j["Text2D"] = GetComponent<Moon::Text2DComponent>()->Serialize();

	for (auto& [name, behaviour] : GetBehaviours()) {
		nlohmann::json behaviourJson;
		behaviourJson["Path"] = Moon::FileSystem::AbsoluteToRelative(behaviour.path);

		auto script = behaviour.GetScript();
		if (script && script->get().IsValid()) {
			sol::table table = *script->get().GetTable();
			for (auto& [key, value] : table) {
				if (key.get_type() == sol::type::string) {
					std::string varName = key.as<std::string>();

					if (value.get_type() == sol::type::number) {
						behaviourJson["Properties"][varName] = value.as<float>();
					}
				}
			}
		}

		j["Behaviours"].push_back(behaviourJson);
	}
	END_SERIALIZE();
}

void Moon::Entity::Deserialize(const json& entity) {
	if (entity.contains("Prefab")) {
		json prefabJson = entity["Prefab"];
		PrefabComponent& prefabComp = AddComponent<PrefabComponent>();
		prefabComp.Deserialize(prefabJson);
	}

	if (entity.contains("Transform")) {
		transform->Deserialize(entity["Transform"]);
	}

	if (entity.contains("Camera")) {
		json cameraJson = entity["Camera"];
		CameraComponent& cc = AddComponent<CameraComponent>();
		cc.Deserialize(cameraJson);
	}

	if (entity.contains("MeshRenderer")) {
		const json& mrc = entity["MeshRenderer"];
		MeshRendererComponent& meshRenderer = AddComponent<MeshRendererComponent>();
		meshRenderer.Deserialize(mrc);
	}

	if (entity.contains("MaterialRenderer")) {
		const json& mrc = entity["MaterialRenderer"];
		MaterialRendererComp& materialRenderer = AddComponent<MaterialRendererComp>();
		MeshRendererComponent* meshRenderer = GetComponent<MeshRendererComponent>();

		if (meshRenderer && meshRenderer->GetModel()) {
			const auto& meshIndices = meshRenderer->GetModel()->GetMeshIndices();

			for (auto meshIndex : meshIndices) {
				Mesh* mesh = AssetManagment::MeshManager::GetMeshByIndex(meshIndex);
				if (!mesh) continue;

				uint32_t matIndex = mesh->materialIndex;

				std::string matPath = "data/engine/materials/default.ftmat";
				if (mrc.contains(std::to_string(matIndex))) {
					std::filesystem::path jsonPath = mrc[std::to_string(matIndex)];
					if (!jsonPath.empty()) {
						std::string candidate = jsonPath.is_absolute() ? jsonPath.string()
							: jsonPath.string().rfind("data", 0) == 0 ? jsonPath.string()
							: FileSystem::RelativeToAbsolute(jsonPath.string());

						if (FileSystem::FileExists(candidate, true)) {
							matPath = candidate;
						}
						else {
							Moon::MessageBox message(
								"Material file not found",
								"Cannot find material at: " + candidate + "\nUsing default material instead.",
								Moon::MessageBox::EMessageType::ERROR
							);
						}
					}
				}
				Rendering::Material* material = Loaders::MaterialLoader::Create(matPath, true);
				if (material)
					materialRenderer.AddMaterialByIndex(matIndex, *material);
			}
		}
	}

	if (entity.contains("PointLight")) {
		json lightJson = entity["PointLight"];
		PointLightComponent& light = AddComponent<PointLightComponent>();
		if (lightJson.contains("Radius"))
			light.SetRadius(lightJson["Radius"]);
		if (lightJson.contains("Strength"))
			light.SetStrength(lightJson["Strength"]);
		if (lightJson.contains("Color"))
			light.SetColor({ lightJson["Color"]["x"], lightJson["Color"]["y"], lightJson["Color"]["z"] });
	}

	if (entity.contains("DirectionalLight")) {
		json lightJson = entity["DirectionalLight"];
		DirectionalLightComp& light = AddComponent<DirectionalLightComp>();
		light.Deserialize(lightJson);
	}

	if (entity.contains("BoxCollider")) {
		json boxColliderJson = entity["BoxCollider"];
		BoxColliderComponent& boxCollider = AddComponent<BoxColliderComponent>();
		boxCollider.Deserialize(boxColliderJson);

		if (entity.contains("RigidBody")) {
			const json& rigidBodyJson = entity["RigidBody"];
			auto rigidBody = GetComponent<RigidBodyComponent>();
			rigidBody->Deserialize(rigidBodyJson);
		}
	}

	if (entity.contains("CharacterController")) {
		json controllerComponent = entity["CharacterController"];
		CharacterControllerComponent& controller = AddComponent<CharacterControllerComponent>();
		controller.GetCharacterController()->SetPosition(transform->GetGlobalPosition());
		controller.GetCharacterController()->SetSettings({
			.height = controllerComponent["Height"],
			.radius = controllerComponent["Radius"]
		});
		//controller.GetCharacterController()->Recreate();
	}

	if (entity.contains("AudioSource")) {
		json audioJson = entity["AudioSource"];
		AudioComponent& audio = AddComponent<AudioComponent>();
		audio.Deserialize(audioJson);
	}

	if (entity.contains("AudioListener")) {
		json audioJson = entity["AudioListener"];
		AudioListenerComponent& audio = AddComponent<AudioListenerComponent>();
		// This is not have to deserialize just add it
	}

	if (entity.contains("Text2D")) {
		json textJson = entity["Text2D"];
		Text2DComponent& text2D = AddComponent<Text2DComponent>();
		text2D.Deserialize(textJson);
	}

	if (entity.contains("Behaviours")) {
		for (auto& behaviourComp : entity["Behaviours"]) {
			std::string path = FileSystem::RelativeToAbsolute(behaviourComp["Path"]);

			if (FileSystem::FileExists(path, true)) {
				Behaviour& behaviour = AddBehaviour(FileSystem::GetFileNameFromPath(path), path);

#ifdef FT_LUASCRIPTING
				auto script = behaviour.GetScript();
				if (script && script->get().IsValid() && behaviourComp.contains("Properties")) {
					sol::table table = *script->get().GetTable();
					for (auto& [propName, propValue] : behaviourComp["Properties"].items()) {
						if (propValue.is_number_float())
							table[propName.c_str()] = propValue.get<float>();
					}
				}
#endif
			}
			else {
				Moon::MessageBox message(
					"Script not found",
					"Behaviour script not found at:\n" + path,
					Moon::MessageBox::EMessageType::ERROR
				);
			}
		}
	}
}

void Moon::Entity::RecursiveActiveUpdate() {

	bool isActive = IsActive();

	if (!m_sleeping)
	{
		if (!m_wasActive && isActive)
		{
			if (!m_awaked)
				OnAwake();

			OnEnable();

			if (!m_started)
				OnStart();
		}

		if (m_wasActive && !isActive)
			OnDisable();
	}

	for (auto child : m_children)
		child->RecursiveActiveUpdate();
}

void Moon::Entity::RecursiveWasActiveUpdate() {
	m_wasActive = IsActive();
	for (auto child : m_children)
		child->RecursiveWasActiveUpdate();
}