#pragma once

#include "AComponent.h"
#include <AssetManagment/Serializable.h>

namespace Moon {

	// TODO: Make PhysicsCapsuleCollider
	class CapsuleColliderComponent : public AComponent {
	public:
		CapsuleColliderComponent(Entity& p_owner);

		json Serialize();

		void Deserialize(const json& j);
	private:
		float radius = 0.5f;
		float height = 1.0f;
	};
}