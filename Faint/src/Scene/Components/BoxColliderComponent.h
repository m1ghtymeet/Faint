#pragma once

//#include "Physics/PhysicsShapes.h"
#include "Component.h"
#include "Math/Transform.h"

namespace Faint {

	class BoxColliderComponent : public Component {
		
		//HAZELCOMPONENT(BoxColliderComponent, "Box Collider");

		static void InitializeComponentClass() {
			//BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
			//BindComponentFieldProperty<&BoxColliderComponent::SetSize>("IsTrigger", "Is Trigger");
		}
	public:
		Vec3 halfExtents = Vec3(0.5f, 0.5f, 0.5f);
		float density = 1.0f;
		Transform shapeOffset;
		bool IsTrigger = true;

		void SetSize(const Vec3& newSize) {
			halfExtents = newSize;
		}

		Vec3 GetSize() {
			return halfExtents;
		}

		json Serialize() {
			BEGIN_SERIALIZE();

			j["IsTrigger"] = IsTrigger;
			//j["Size"] = Size;
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			this->IsTrigger = j["IsTrigger"];
			this->halfExtents = Vec3(j["Size"]["x"], j["Size"]["y"], j["Size"]["z"]);
			return true;
		}
	};
}