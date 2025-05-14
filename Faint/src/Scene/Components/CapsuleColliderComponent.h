#pragma once

#include "Component.h"
//#include "Physics/PhysicsShapes.h"
//#include "Core/Object/Object.h"

namespace Faint {

	struct CapsuleColliderComponent : public Component {
		//FAINTCOMPONENT(BoxColliderComponent, "Box Collider");
	public:
		//Ref<Physics::Capsule> capsule;
		float radius = 0.5f;
		float height = 1.0f;
		bool isTrigger = false;
		json Serialize() {
			BEGIN_SERIALIZE();
			j["IsTrigger"] = isTrigger;
			SERIALIZE_VAL(radius);
			SERIALIZE_VAL(height);
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			this->isTrigger = j["IsTrigger"];
			this->radius = j["radius"];
			this->height = j["height"];
			return true;
		}
	};
}