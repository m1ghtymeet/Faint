#pragma once

#define FT_LUASCRIPTING 1

#ifdef FT_LUASCRIPTING
#include "Lua/LuaScriptEngine.h"
#include "Lua/LuaScript.h"
#endif

namespace Moon::Scripting {

#ifdef FT_LUASCRIPTING
	using Script = Moon::Scripting::LuaScript;
	using ScriptEngine = Moon::Scripting::LuaScriptEngine;
#elif FT_CSHARPSCRIPTING
	using Script;
	using ScriptEngine;
#endif
}