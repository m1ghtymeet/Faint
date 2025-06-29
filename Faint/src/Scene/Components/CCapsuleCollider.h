#pragma once

#include "AComponent.h"

namespace Faint {

	struct CapsuleColliderComponent : public AComponent {
	public:
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