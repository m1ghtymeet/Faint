#include "LuaScriptEngine.h"
#include <Debug/Assertion.h>
#include <FileSystem/FileSystem.h>

#include <Scene/Components/Behaviour.h>
#include <Scene/Components/CCollider.h>

#include <sol/sol.hpp>

void BindLuaCore(sol::state& p_state);
void BindLuaEntity(sol::state& p_state);
void BindLuaComponents(sol::state& p_state);
void BindLuaGlobal(sol::state& p_state);
void BindLuaMath(sol::state& p_state);
void BindLuaPhysics(sol::state& p_state);

constexpr auto luaBindings = std::array{
	BindLuaCore,
	BindLuaEntity,
	BindLuaComponents,
	BindLuaGlobal,
	BindLuaMath,
	BindLuaPhysics
};

template<typename... Args>
static void ExecuteLuaFunction(Moon::Behaviour& p_behaviour, const std::string& p_functionName, Args&&... p_args) {
	auto context = p_behaviour.GetScript();
	if (!context || !context->get().IsValid()) return;

	//FT_CORE_ASSERT(context->get().IsValid(), "The given context is invalid");

	sol::table& self = *static_cast<Moon::Scripting::LuaScript&>(context.value()).GetTable();
	sol::protected_function fn = self[p_functionName];
	if (!fn.valid()) return;

	auto pfrResult = fn(self, std::forward<Args>(p_args)...);
	if (!pfrResult.valid()) {
		sol::error err = pfrResult;
		HZ_CORE_ERROR("[Lua] {}", err.what());
	}
}

sol::table LoadScript(sol::state& p_luaState, const std::string& p_scriptName) {

	//sol::table packageTable = p_luaState.get<sol::table>("package");
	//std::string currentPath = packageTable.get<std::string>("path");
	std::string newPath = Moon::FileSystem::Root + "?.lua";
	p_luaState.safe_script("package.path = \"" + newPath + "\"", &sol::script_pass_on_error);
	const auto result = p_luaState.safe_script_file(p_scriptName, &sol::script_pass_on_error);

	if (!result.valid()) {
		sol::error err = result;
		HZ_CORE_ERROR(err.what());
		return {};
	}
	else {
		if (result.return_count() == 1 && result[0].is<sol::table>())
			return result[0];
		else {
			HZ_CORE_ERROR("'" + p_scriptName + "' missing return expression");
			return {};
		}
	}
}

bool RegisterBehaviour(sol::state& p_luaState, Moon::Behaviour& p_behaviour, const std::string& p_scriptName) {

	auto table = LoadScript(p_luaState, p_scriptName);

	p_behaviour.SetScript(std::make_unique<Moon::Scripting::LuaScript>(table));

	if (auto context = p_behaviour.GetScript(); context.has_value()) {
		auto& luaScript = static_cast<Moon::Scripting::LuaScript&>(context.value());
		luaScript.SetOwner(p_behaviour.owner);
		return true;
	}

	return false;
}

int Moon::Scripting::LuaScriptEngine::LuaPanicHandler(lua_State* l) {
	const char* msg = lua_tostring(l, -1);
	HZ_CORE_TRACE("[LUA PANIC] {}", msg ? msg : "unknown error");
	return 0;
}

int Moon::Scripting::LuaScriptEngine::LuaErrorHandler(lua_State* l, sol::optional<const std::exception&> maybe_exception, std::string_view description) {
	HZ_CORE_ERROR("=== LUA EXCEPTION ===");

	if (maybe_exception)
	{
		HZ_CORE_ERROR("[Exception] {}", maybe_exception->what());
	}
	else
	{
		HZ_CORE_ERROR("[Error] {}", description);
	}

	luaL_traceback(l, l, nullptr, 1);
	const char* traceback = lua_tostring(l, -1);
	if (traceback)
	{
		HZ_CORE_ERROR("Stack Trace:\n{}", traceback);
		lua_pop(l, 1); // Å«ò ò—œ‰ traceback
	}

	HZ_CORE_ERROR("====================");
	return 0;
}

void Moon::Scripting::LuaScriptEngine::SetupErrorHandling() {
	sol::state_view state(*luaState);

	state["print"] = [this](sol::variadic_args va) {
		std::ostringstream output;
		bool first = true;
		for (auto v : va) {
			if (!first) output << "\t";
			lua_State* l = va.lua_state();
			lua_pushvalue(l, v.stack_index());
			const char* str = lua_tostring(l, -1);
			output << (str ? str : "nil");
			lua_pop(l, 1);
			first = false;
		}
		
		lua_Debug ar;
		if (lua_getstack(luaState->lua_state(), 1, &ar) && lua_getinfo(luaState->lua_state(), "Sl", &ar)) {
			std::string source = ar.short_src ? ar.short_src : "unknown";
			int line = ar.currentline > 0 ? ar.currentline : -1;
			if (line != -1) {
				HZ_CORE_TRACE("[Lua:{}:{}] {}", source, line, output.str());
			}
			else {
				HZ_CORE_TRACE("[Lua:{}] {}", source, output.str());
			}
		}
		else {
			HZ_CORE_TRACE("[Lua] {}", output.str());
		}
		};

	/*state["xpcall"] = [this](sol::function fn, sol::variadic_args args) -> sol::variadic_results {
		sol::protected_function pfn(fn);
		pfn.set_error_handler(sol::c_call<decltype(&LuaErrorHandler), &LuaErrorHandler>);
		return { pfn(args) };
		};*/
}

Moon::Scripting::LuaScriptEngine::LuaScriptEngine() {
	CreateContext();
}

Moon::Scripting::LuaScriptEngine::~LuaScriptEngine() {
	DestroyContext();
}

void Moon::Scripting::LuaScriptEngine::CreateContext() {
	luaState = std::make_unique<sol::state>();
	luaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::table);
	luaState->set_exception_handler(&LuaErrorHandler);
	luaState->set_panic(sol::c_call<decltype(&LuaPanicHandler), &LuaPanicHandler>);

	SetupErrorHandling();
	for (auto& callback : luaBindings) {
		callback(*luaState);
	}

	errorCount = 0;

	std::for_each(behaviours.begin(), behaviours.end(),
		[this](std::reference_wrapper<Behaviour> behaviour) {
			const auto scriptFilename = behaviour.get().path /* + GetDefaultExtension()*/;
			const auto scriptPath = scriptFilename;
			if (!RegisterBehaviour(*luaState, behaviour.get(), scriptPath))
				errorCount++;
		}
	);

	if (errorCount > 0) {
		const std::string message = std::to_string(errorCount) + " script(s) failed to register";
		HZ_CORE_ERROR(message);
	}
}

void Moon::Scripting::LuaScriptEngine::DestroyContext() {

	FT_CORE_ASSERT(luaState != nullptr, "No valid Lua context");

	std::for_each(behaviours.begin(), behaviours.end(),
		[this](std::reference_wrapper<Behaviour> behaviour) {
			behaviour.get().RemoveScript();
		}
	);

	luaState.reset();
}

void Moon::Scripting::LuaScriptEngine::HotReloadIfNeeded() {

}

std::string Moon::Scripting::LuaScriptEngine::GetDefaultScriptContent(const std::string& p_name) {
	return "local " + p_name + " =\n{\n}\n\nfunction " + p_name + ":Start()\nend\nfunction " + p_name + ":Update(deltaTime)\nend\nreturn " + p_name;
}

std::string Moon::Scripting::LuaScriptEngine::GetDefaultExtension() {
	return ".lua";
}

void Moon::Scripting::LuaScriptEngine::AddBehaviour(Behaviour& p_toAdd) {
	FT_CORE_ASSERT(luaState != nullptr, "No valid Lua context");

	behaviours.push_back(std::ref(p_toAdd));

	if (!RegisterBehaviour(*luaState, p_toAdd, p_toAdd.path)) {
		errorCount++;
	}
}

void Moon::Scripting::LuaScriptEngine::RemoveBehaviour(Behaviour& p_toRemove) {

	if (luaState)
		p_toRemove.RemoveScript();

	behaviours.erase(
		std::remove_if(behaviours.begin(), behaviours.end(),
			[&p_toRemove](std::reference_wrapper<Behaviour> behaviour) {
				return &p_toRemove == &behaviour.get();
			}
		)
	);

	Reload();
}

void Moon::Scripting::LuaScriptEngine::Reload() {
	static_cast<LuaScriptEngine&>(*this).DestroyContext();
	static_cast<LuaScriptEngine&>(*this).CreateContext();
}

bool Moon::Scripting::LuaScriptEngine::IsOk() const {
	return luaState && errorCount == 0;
}

void Moon::Scripting::LuaScriptEngine::OnAwake(Behaviour& p_target) {
	ExecuteLuaFunction(p_target, "Awake");
}

void Moon::Scripting::LuaScriptEngine::OnStart(Behaviour& p_target) {
	ExecuteLuaFunction(p_target, "Start");
}

void Moon::Scripting::LuaScriptEngine::OnEnable(Behaviour& p_target) {
	ExecuteLuaFunction(p_target, "OnEnable");
}

void Moon::Scripting::LuaScriptEngine::OnUpdate(Behaviour& p_target, float p_deltaTime) {
	ExecuteLuaFunction(p_target, "Update", p_deltaTime);
}

void Moon::Scripting::LuaScriptEngine::OnTriggerEnter(Behaviour& p_target, ColliderComponent& otherBody) {
	ExecuteLuaFunction(p_target, "OnTriggerEnter", otherBody);
}

void Moon::Scripting::LuaScriptEngine::OnTriggerStay(Behaviour& p_target, ColliderComponent& otherBody) {
	ExecuteLuaFunction(p_target, "OnTriggerStay", otherBody);
}

void Moon::Scripting::LuaScriptEngine::OnTriggerExit(Behaviour& p_target, ColliderComponent& otherBody) {
	ExecuteLuaFunction(p_target, "OnTriggerExit", otherBody);
}
