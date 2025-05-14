#pragma once

#include "Component.h"

#include "Core/Base.h"
#include "Math/Math.h"

#include "Renderer/Types/Material.h"
#include "Renderer/Types/Mesh.h"
#include "Physics/Types/RigidDynamic.h"

namespace Faint {

	class BSPBrushComponent : public Component {
	public:
		std::vector<std::vector<Vec3>> Hulls;

		std::vector<Ref<Material>> Materials;
		std::vector<Ref<RigidDynamic>> Rigidbody;

		std::string target = "";
		std::string TargetName = "";
		//std::vector<Entity> Targets;

		bool IsSolid = true;
		bool IsTrigger = false;
		bool IsTransparent = false;
		bool IsFunc = false;

		BSPBrushComponent() {
			Materials = std::vector<Ref<Material>>();
			//Rigidbody = std::vector<Ref<RigidDynamic>>();
			Hulls = std::vector<std::vector<Vec3>>();
		}

		json Serialize() {

			BEGIN_SERIALIZE();

			for (uint32_t i = 0; i < Hulls.size(); i++) {
				json hullPointsJson;

				j["Hulls"][i] = hullPointsJson;
			}

			END_SERIALIZE();
		}

		bool Deserialize() {
			return true;
		}
	};
}