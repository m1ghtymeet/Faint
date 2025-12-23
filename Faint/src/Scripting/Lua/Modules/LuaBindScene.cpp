
#include <sol/sol.hpp>
#include <Scripting/ScriptEngine.h>
#include <Debug/Log.h>

#include <Core/GlobalLocator.h>
#include <FileSystem/FileSystem.h>
#include <Scene/Entity.h>
#include <Scene/SceneManager.h>
#include <Scene/Components.h>

void BindLuaEntity(sol::state& p_state) {

	using namespace Moon;

	// ===----=== Entity ===----===
	auto type = p_state.new_usertype<Entity>("Entity",
		/* Methods */
		"GetName", &Entity::GetName,
		"SetName", &Entity::SetName,
		"GetTag", &Entity::GetTag,
		"SetTag", &Entity::SetTag,
		"Children", &Entity::GetChildren,
		"SetActive", &Entity::SetActive,
		"IsActive", &Entity::IsActive,
		"GetID", &Entity::GetID,
		"Destroy", &Entity::Destroy,
		"Parent", &Entity::GetParent,
		"SetParent", &Entity::SetParent,
		"GetChild", sol::readonly_property(&Entity::GetChildByName),
		"GetChildByName", &Entity::GetChildByName,

		/* Components */
		"GetComponent", [&](Entity& self, const std::string& componentName) -> sol::object {
			#define TRY_COMPONENT(Type, Name) \
				if (componentName == Name) { \
					if (auto* comp = self.GetComponent<Type>()) \
						return sol::make_object(p_state, comp); \
				}
			TRY_COMPONENT(TransformComponent, "Transform")
			TRY_COMPONENT(CameraComponent, "Camera")
			TRY_COMPONENT(RigidBodyComponent, "Rigidbody")
			TRY_COMPONENT(CharacterControllerComponent, "CharacterController")
			TRY_COMPONENT(AudioComponent, "AudioSource")
			TRY_COMPONENT(Text2DComponent, "Text2D")
			TRY_COMPONENT(MeshRendererComponent, "MeshRenderer")
			TRY_COMPONENT(BoxColliderComponent, "BoxCollider")
			TRY_COMPONENT(PointLightComponent, "PointLight")
			TRY_COMPONENT(DirectionalLightComp, "DirectionalLight")
			//TRY_COMPONENT(MaterialRendererComp, "Text2D")

			#undef TRY_COMPONENT
			return sol::nil;
		},
		"AddComponent", [&](Entity& self, const std::string& componentName) {
			#define TRY_ADD_COMPONENT(Type, Name) \
				if (componentName == Name) { \
					self.AddComponent<Type>(); \
					return; \
				}
			TRY_ADD_COMPONENT(TransformComponent, "Transform")
			TRY_ADD_COMPONENT(CameraComponent, "Camera")
			TRY_ADD_COMPONENT(RigidBodyComponent, "Rigidbody")
			TRY_ADD_COMPONENT(CharacterControllerComponent, "CharacterController")
			TRY_ADD_COMPONENT(AudioComponent, "AudioSource")
			TRY_ADD_COMPONENT(Text2DComponent, "Text2D")
			TRY_ADD_COMPONENT(MeshRendererComponent, "MeshRenderer")
			TRY_ADD_COMPONENT(BoxColliderComponent, "BoxCollider")
			TRY_ADD_COMPONENT(PointLightComponent, "PointLight")
			TRY_ADD_COMPONENT(DirectionalLightComp, "DirectionalLight")
			//TRY_COMPONENT(MaterialRendererComp, "Text2D")

			#undef TRY_ADD_COMPONENT
			HZ_CORE_WARN("Component not found: " + componentName);
		},

		/* Behaviours */
		"AddBehaviour", &Entity::AddBehaviour,
		"RemoveBehaviour", sol::overload(
			sol::resolve<bool(Behaviour&)>(&Entity::RemoveBehaviour),
			sol::resolve<bool(const std::string&)>(&Entity::RemoveBehaviour)
		),
		"GetBehaviour", [](Entity& p_this, const std::string& p_name) -> sol::table {
			if (auto behaviour = p_this.GetBehaviour(p_name)) {
				if (auto script = behaviour->GetScript()) {
					return *static_cast<Scripting::LuaScript&>(script.value()).GetTable();
				}
			}
		}
	);

	// ===-----=== Scene ===-----===
	p_state.new_usertype<Scene>("Scene",
		"GetEntityByName", &Scene::GetEntityByName,
		"GetEntityByTag", &Scene::GetEntityByTag,
		"GetEntityByID", &Scene::GetEntityByID,
		"CreateEntity", sol::overload(
			sol::resolve<Entity&(const std::string&, const std::string&)>(&Scene::CreateEntity)
		),
		"DestroyEntity", &Scene::DestroyEntity,
		"InstantiatePrefab", [](const std::string& path) -> Entity& {
			return FTSERVICE(SceneManager).GetCurrentScene()->InstantiatePrefab(FileSystem::Root + path);
		}
	);
}