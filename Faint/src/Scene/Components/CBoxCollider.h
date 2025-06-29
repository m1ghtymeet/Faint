#pragma once

//#include "Physics/PhysicsShapes.h"
#include "AComponent.h"
#include "Math/Transform.h"

namespace Faint {
	class BoxColliderComponent : public AComponent {
	public:
		BoxColliderComponent(Entity& p_owner) : AComponent(p_owner) {}
		
		std::string GetName() override { return "BoxCollider"; }
		
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
			SERIALIZE_VEC3(halfExtents);
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			this->IsTrigger = j["IsTrigger"];
			this->halfExtents = Vec3(j["Size"]["x"], j["Size"]["y"], j["Size"]["z"]);
			return true;
		}
	};
}