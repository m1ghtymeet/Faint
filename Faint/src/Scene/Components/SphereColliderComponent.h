#pragma once

//#include "Physics/PhysicsShapes.h"
#include "Component.h"

namespace Faint {

	class SphereColliderComponent : public Component {
		
		//HAZELCOMPONENT(BoxColliderComponent, "Box Collider");

		static void InitializeComponentClass() {
			//BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
			//BindComponentFieldProperty<&BoxColliderComponent::SetSize>("IsTrigger", "Is Trigger");
		}
	public:
		//Ref<Physics::Sphere> Sphere;
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