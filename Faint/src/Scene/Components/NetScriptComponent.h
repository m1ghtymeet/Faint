#pragma once

#include "AComponent.h"

#include "Core/Base.h"
#include "Math/Math.h"
#include "AssetManagment/Serializable.h"

namespace Faint {
	enum class NetScriptExposedVarType {
		Bool,
		Float,
		Double,
		String,
		Entity,
		Prefab,
		Vec2,
		Vec3,
		Vec4
	};
	struct NetScriptExposedVar {
		std::string Name;
		std::any Value;
		std::any DefaultValue;
		NetScriptExposedVarType Type;
	};

	class NetScriptComponent : public AComponent {
	public:
		std::string ScriptPath;
		std::string Class;
		NetScriptComponent(Entity& p_owner) : AComponent(p_owner) {}

		std::string GetName() override { return "NetScript"; }

		bool Initialized = false;

		std::vector<NetScriptExposedVar> ExposedVar;

		json Serialize() {
			BEGIN_SERIALIZE();
			j["Path"] = ScriptPath;
			j["Class"] = Class;
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			
			return true;
		}
	};
}