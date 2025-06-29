#pragma once

#include "AComponent.h"
#include "CTransform.h"

#include "Physics/Physics.h"
#include "Core/Base.h"

namespace Faint {
	/**
		- Mass * Float
		- Drag * Float
		- Angular Drag * Float
		- Use Gravity * Bool
		- Is Kinematic * Bool
		- Collision Detection * Layer
		Constraints:
			- Freez Position * Vec3
			- Freez Rotation * Vec3
	*/
	struct RigidBodyComponent : public AComponent {
	public:
		PxRigidActor* actor = nullptr;
		bool useGravity = true;
		bool isKinematic = false;
		float mass;
		bool LockPosX = false; bool LockPosY = false; bool LockPosZ = false;
		bool LockRotX = false; bool LockRotY = false; bool LockRotZ = false;

		RigidBodyComponent(Entity& p_owner);

		std::string GetName() override { return "RigidBody"; }

		PxRigidDynamic* GetDynamicBody() const {
			if (actor && actor->is<PxRigidDynamic>()) {
				return static_cast<PxRigidDynamic*>(actor);
			}
			return nullptr;
		}
		PxRigidStatic* GetStaticBody() const {
			if (actor && actor->is<PxRigidStatic>()) {
				return static_cast<PxRigidStatic*>(actor);
			}
			return nullptr;
		}

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(useGravity);
			SERIALIZE_VAL(isKinematic);
			SERIALIZE_VAL(mass);
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			return true;
		}
	};
}