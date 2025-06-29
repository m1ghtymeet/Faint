#pragma once

//#include "Physics/PhysicsShapes.h"
#include "AComponent.h"

namespace Faint {

	class SphereColliderComponent : public AComponent {
	public:
		SphereColliderComponent(Entity& p_owner) : AComponent(p_owner) {}

		std::string GetName() override { return "SphereCollider"; }

		float Radius = 0.5f;
		bool IsTrigger = false;

		void SetRadius(const float newRadius) {
			Radius = newRadius;
		}

		float GetRadius() {
			return Radius;
		}

		json Serialize() {
			BEGIN_SERIALIZE();

			j["IsTrigger"] = IsTrigger;
			j["Radius"] = Radius;
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			this->IsTrigger = j["IsTrigger"];
			this->Radius = j["Radius"];
			return true;
		}
	};
}