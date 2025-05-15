#pragma once

#include "Component.h"
#include "AssetManagment/Serializable.h"
#include <sol/sol.hpp>

namespace Faint {
	class LuaScriptComponent : public Component {
	public:
		std::string path;
		sol::environment env;
		sol::table instance;

		json Serialize() {
			BEGIN_SERIALIZE();
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {

			
			return true;
		}
	};
}