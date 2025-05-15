#pragma once
#include <sol/sol.hpp>

#include "Scene/Entity.h"
#include "Engine.h"

#include "Input/Input.h"

namespace Faint {
	class ScriptEngine {
	public:
		static void Init();

		static std::unique_ptr<sol::state> luaState;

		static void RunScript(const std::string& filepath) {
			luaState->script_file(filepath);
		}
	};
}