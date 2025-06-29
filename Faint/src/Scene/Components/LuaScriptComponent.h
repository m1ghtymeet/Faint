#pragma once

#include "AComponent.h"
#include "AssetManagment/Serializable.h"
#include <sol/sol.hpp>

namespace Faint {
	class LuaScriptComponent : public AComponent {
	public:
		std::string path;
		sol::environment env;
		sol::table instance;

		json Serialize() {
			BEGIN_SERIALIZE();
			j["path"] = path;
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			if (j.contains("path"))
				path = j["path"];
			return true;
		}
	};
}