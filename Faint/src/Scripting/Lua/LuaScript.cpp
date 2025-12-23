#include "LuaScript.h"
#include <Scene/Entity.h>
#include <sol/sol.hpp>

Moon::Scripting::LuaScript::LuaScript(sol::table p_table) {
    table = std::make_unique<sol::table>(p_table);
}

Moon::Scripting::LuaScript::~LuaScript()
{
}

void Moon::Scripting::LuaScript::SetOwner(Entity& p_owner) {
    (*table)["owner"] = &p_owner;
}

bool Moon::Scripting::LuaScript::IsValid() const {
    return table && table->valid();
}
