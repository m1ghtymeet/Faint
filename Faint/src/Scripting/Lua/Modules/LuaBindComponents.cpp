
#include <sol/sol.hpp>

#include <Math/Math.h>
#include <Scene/Scene.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <Scene/Components/CBoxCollider.h>
#include <Scene/Components/CCharacterController.h>
#include <Physics/CharacterController.h>

void BindLuaComponents(sol::state& p_state) {

	using namespace Moon;
	using namespace Moon::Math;

	p_state.new_usertype<AComponent>("Component",
		"GetOwner", [](AComponent& p_component) -> Entity& { return p_component.owner; }
	);

	p_state.new_usertype<TransformComponent>("Transform",
		sol::base_classes, sol::bases<AComponent>(),
		"SetPosition", [](TransformComponent& p_this, Vector3 position) {
			p_this.SetLocalPosition({ position.x, position.y, position.z });
		},
		"GetPosition", [](TransformComponent& p_this) -> Vector3 { return { p_this.GetLocalPosition().x, p_this.GetLocalPosition().y, p_this.GetLocalPosition().z }; },
		"SetLocalPosition", [](TransformComponent& p_this, Vector3 position) {
			p_this.SetLocalPosition({ position.x, position.y, position.z });
		},
		"SetLocalRotation", [](TransformComponent& p_this, Vector3 rotation) {
			glm::vec3 rotationRad = glm::radians(glm::vec3(rotation.x, rotation.y, rotation.z));
			glm::quat newRotation = glm::quat(rotationRad);
			p_this.SetLocalRotation(newRotation);
		},
		"SetLocalScale", &TransformComponent::SetLocalScale,
		"GetLocalPosition", [](TransformComponent& p_this) -> Vector3 {
			return { p_this.GetLocalPosition().x, p_this.GetLocalPosition().y, p_this.GetLocalPosition().z };
		},
		"GetLocalRotation", [](TransformComponent& p_this) -> Vector3 {
			glm::vec3 euler = glm::eulerAngles(p_this.GetLocalRotation());
			return { euler.x, euler.y, euler.z };
		},
		"SetWorldPosition", [](TransformComponent& p_this, Vector3 value) {
			p_this.SetGlobalPosition({ value.x, value.y, value.z });
		},
		"SetWorldRotation", [](TransformComponent& p_this, Vector3 value) {
			p_this.SetGlobalRotation(glm::quat(glm::vec3(value.x, value.y, value.z)));
		},
		"SetWorldScale", &TransformComponent::SetGlobalScale,
		"GetWorldPosition", [](TransformComponent& p_this) -> Vector3 {
			return { p_this.GetGlobalPosition().x, p_this.GetGlobalPosition().y, p_this.GetGlobalPosition().z };
		},
		"ToForwardVector", [](TransformComponent& p_this) -> Vector3 {
			glm::quat q = p_this.GetGlobalRotation();
			glm::vec3 forward = glm::normalize(q * glm::vec3(0.0f, 0.0f, 1.0f));
			return { forward.x, forward.y, forward.z };
		},
		"ToRightVector", [](TransformComponent& p_this) -> Vector3 {
			glm::quat q = p_this.GetGlobalRotation();
			glm::vec3 right = glm::normalize(q * glm::vec3(1.0f, 0.0f, 0.0f));
			return { right.x, right.y, right.z };
		},
		"ToUpVector", [](TransformComponent& p_this) -> Vector3 {
			glm::quat q = p_this.GetGlobalRotation();
			glm::vec3 up = glm::normalize(q * glm::vec3(0.0f, 1.0f, 0.0f));
			return { up.x, up.y, up.z };
		}
	);

	p_state.new_usertype<CameraComponent>("Camera",
		sol::base_classes, sol::bases<AComponent>(),
		"SetNear", &CameraComponent::SetNear,
		"SetFar", &CameraComponent::SetFar,
		"SetFov", &CameraComponent::SetFov,
		"GetNear", &CameraComponent::GetNear,
		"GetFar", &CameraComponent::GetFar,
		"GetFov", &CameraComponent::GetFov,
		"GetForward", [](CameraComponent& p_this) -> Vector3 { return { p_this.camera.GetForward().x, p_this.camera.GetForward().y, p_this.camera.GetForward().z }; },
		"GetRight", [](CameraComponent& p_this) -> Vector3 { return { p_this.camera.GetRight().x, p_this.camera.GetRight().y, p_this.camera.GetRight().z }; },
		"GetUp", [](CameraComponent& p_this) -> Vector3 { return { p_this.camera.GetUp().x, p_this.camera.GetUp().y, p_this.camera.GetUp().z }; }
	);

	p_state.new_usertype<MeshRendererComponent>("MeshRenderer",
		sol::base_classes, sol::bases<AComponent>(),
		"GetModel", &MeshRendererComponent::GetModel
	);

	p_state.new_usertype<MaterialRendererComp>("MaterialRenderer",
		sol::base_classes, sol::bases<AComponent>(),
		"GetMaterialByIndex", &MaterialRendererComp::GetMaterialByIndex
	);

	p_state.new_usertype<ColliderComponent>("Collider",
		sol::base_classes, sol::bases<AComponent>()
	);

	p_state.new_usertype<RigidBodyComponent>("RigidBody",
		sol::base_classes, sol::bases<AComponent>(),
		"SetMass", &RigidBodyComponent::SetMass,
		"GetMass", &RigidBodyComponent::GetMass,
		"SetTrigger", &RigidBodyComponent::SetTrigger,
		"IsTrigger", &RigidBodyComponent::IsTrigger,
		"IsKinematic", &RigidBodyComponent::IsKinematic,
		"SetKinematic", &RigidBodyComponent::SetKinematic,

		// Advanced properties
		"SetLinearDrag", &RigidBodyComponent::SetLinearDrag,
		"GetLinearDrag", &RigidBodyComponent::GetLinearDrag,
		"SetAngularDrag", &RigidBodyComponent::SetAngularDrag,
		"GetAngularDrag", &RigidBodyComponent::GetAngularDrag,
		"SetGravityScale", &RigidBodyComponent::SetGravityScale,
		"GetGravityScale", &RigidBodyComponent::GetGravityScale,

		// Motion constraints
		"SetFreezePosition", [](RigidBodyComponent& p_this, Vector3 freezeValue) {
			glm::bvec3 freeze = {
				freezeValue.x,
				freezeValue.y,
				freezeValue.z
			};
			p_this.SetFreezePosition(freeze);
		},
		"GetFreezePosition", [](RigidBodyComponent& p_this) -> Vector3 {
			auto freeze = p_this.GetFreezePosition();
			return { static_cast<float>(freeze.x), static_cast<float>(freeze.y), static_cast<float>(freeze.z) };
		},
		// FreezeRotation will be added

		// Sleep properties
		"SetEnableSleep", &RigidBodyComponent::SetEnableSleep,
		"GetEnableSleep", &RigidBodyComponent::GetEnableSleep,
		"SetSleepThreshold", &RigidBodyComponent::SetSleepThreshold,
		"GetSleepThreshold", &RigidBodyComponent::GetSleepThreshold,

		// Physics material
		"SetBounciness", &RigidBodyComponent::SetBounciness,
		"GetBounciness", &RigidBodyComponent::GetBounciness,
		"SetFriction", &RigidBodyComponent::SetFriction,
		"GetFriction", &RigidBodyComponent::GetFriction,

		"AddForce", [](RigidBodyComponent& p_this, Vector3 force) {
			p_this.AddForce({ force.x, force.y, force.z });
		},
		
		"MovePosition", [](RigidBodyComponent& p_this, Vector3 position) {
			p_this.MovePosition({ position.x, position.y, position.z });
		}
		// TODO: Add quaternion or change quaternion to Vector3
		//"MoveRotation", [](RigidBodyComponent& p_this, Vector3 position) {
		//	p_this.MovePosition({ position.x, position.y, position.z });
		//}
	);

	p_state.new_usertype<BoxColliderComponent>("BoxCollider",
		sol::base_classes, sol::bases<AComponent>()
	);

	p_state.new_usertype<CharacterControllerComponent>("CharacterController",
		sol::base_classes, sol::bases<AComponent>(),
		"Move", [](CharacterControllerComponent& p_this, Vector3 velocity) -> void { p_this.GetCharacterController()->Move({ velocity.x, velocity.y, velocity.z }); },
		"IsGrounded", &CharacterControllerComponent::IsGrounded,
		"GetFootPosition", [](CharacterControllerComponent& p_this) -> Vector3 { return { p_this.GetCharacterController()->GetFootPosition().x, p_this.GetCharacterController()->GetFootPosition().y, p_this.GetCharacterController()->GetFootPosition().z}; }
	);

	p_state.new_usertype<AudioComponent>("AudioSource",
		sol::base_classes, sol::bases<AComponent>(),
		"filepath", &AudioComponent::filepath,
		"Play", &AudioComponent::Play,
		"Stop", &AudioComponent::Stop
	);

	p_state.new_usertype<Text2DComponent>("Text2D",
		sol::base_classes, sol::bases<AComponent>(),
		"SetText", &Text2DComponent::SetText,
		"GetText", &Text2DComponent::GetText
	);
}