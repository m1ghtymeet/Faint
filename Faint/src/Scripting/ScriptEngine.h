#pragma once
#include <sol/sol.hpp>

#include "Scene/Entity.h"
#include "Engine.h"

#include "Input/Input.h"

namespace Faint {
	class ScriptEngine {
	public:
		static void Init();
		static void Shutdown();

		static sol::state& GetLuaState() { return *luaState; }

	private:
		static std::unique_ptr<sol::state> luaState;

		static void BindCoreTypes();
		static void BindInput();
		static void BindEntity();
		static void BindComponents();
	};
}