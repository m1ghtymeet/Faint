#pragma once

#include <filesystem>
#include <memory>
#include <vector>

struct lua_State;

namespace Moon {
	class Behaviour;
	class ColliderComponent;
}

namespace sol {
	class state;
	template <class T>
	class optional;
}

namespace Moon::Scripting {

	class LuaScriptEngine {
	private:
		std::unique_ptr<sol::state> luaState;
		std::filesystem::path scriptRootFolder;

		std::vector<std::reference_wrapper<Behaviour>> behaviours;
		uint32_t errorCount;

		static int LuaPanicHandler(lua_State* l);
		static int LuaErrorHandler(lua_State* l, sol::optional<const std::exception&> maybe_exception, std::string_view description);

		void SetupErrorHandling();

	public:
		LuaScriptEngine();
		//~LuaScriptEngine(const LuaScriptEngine&) = delete;
		LuaScriptEngine& operator=(const LuaScriptEngine&) = delete;
		virtual ~LuaScriptEngine();

		void CreateContext();
		void DestroyContext();
		void HotReloadIfNeeded();

		/* Base */
		std::vector<std::string> GetValidExtensions();

		std::string GetDefaultScriptContent(const std::string& p_name);

		std::string GetDefaultExtension();

		void Reload();

		bool IsOk() const;

		// Behaviour Management
		void AddBehaviour(Behaviour& p_toAdd);
		void RemoveBehaviour(Behaviour& p_toRemove);

		// Callbacks
		void OnAwake(Behaviour& p_target);
		void OnStart(Behaviour& p_target);
		void OnEnable(Behaviour& p_target);
		void OnUpdate(Behaviour& p_target, float p_deltaTime);
		void OnTriggerEnter(Behaviour& p_target, ColliderComponent& otherBody);
		void OnTriggerStay(Behaviour& p_target, ColliderComponent& otherBody);
		void OnTriggerExit(Behaviour& p_target, ColliderComponent& otherBody);
		
		// Debug & Utils
		void PrintLuaStack();
		void CollectGrabage();
	};
}