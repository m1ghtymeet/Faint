#pragma once
#include "AComponent.h"
#include "Physics/Physics.h"

namespace Faint {
	/**
		- Shape Limit
		- Step offset
		- Skin Width
		- Min Move Distance
		- Center XYZ
		- Radius
		- Height
	*/
	class CharacterControllerComponent : public AComponent {
	public:
		CharacterControllerComponent(Entity& p_owner) : AComponent(p_owner) {}
		~CharacterControllerComponent() = default;

		std::string GetName() override { return "CharacterController"; }

		PxController* controller = nullptr;
		PxExtendedVec3 position = PxExtendedVec3(0, 0, 0);
		float slopeLimit = 0.7f;
		float stepOffset = 0.2f;
		float skinWidth = 0.08f;
		float minMoveDistance = 0.0f;
		float radius = 0.5f;
		float height = 1.8f;

		void Create() {
			PxCapsuleControllerDesc* desc = new PxCapsuleControllerDesc;
			desc->setToDefault();
			desc->height = height;
			desc->radius = radius;
			desc->position = position;
			desc->material = Physics::GetDefaultMaterial();
			desc->stepOffset = stepOffset;
			desc->contactOffset = 0.001f;
			desc->slopeLimit = slopeLimit;

			if (!controller) {
				controller = Physics::GetCharacterControllerManager()->createController(*desc);
				PxFilterData filterData;
				filterData.word0 = RAYCAST_DISABLED;
				filterData.word1 = CollisionGroup::CHARACTER_CONTROLLER;
				filterData.word2 = CollisionGroup::DYNAMIC_OBJECT | CollisionGroup::STATIC_OBJECT;
			}
		}

		void Move(glm::vec3 velocity) {
			if (controller) {
				PxFilterData filterData;
				filterData.word0 = 0;
				filterData.word1 = CollisionGroup::DYNAMIC_OBJECT | CollisionGroup::STATIC_OBJECT;
				PxControllerFilters data;
				PxF32 minDist = 0.001f;
				data.mFilterData = &filterData;
				float fixedDeltaTime = (1.0f / 60.0f);
				controller->move(Physics::GlmVec3toPxVec3(velocity), minDist, fixedDeltaTime, data);
			}
		}

		bool OnGrounded(PxController* controller) {
			return false;
		}
	};
}