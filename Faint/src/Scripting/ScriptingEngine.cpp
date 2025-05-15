#include "ScriptingEngine.h"
#include "WrenScript.h"

#include "Engine.h"
#include "FileSystem/FileSystem.h"

#include "Modules/ScriptModule.h"
#include "Modules/EngineModule.h"

namespace Faint {

	WrenVM* ScriptingEngine::m_WrenVM;

	std::map<std::string, Ref<WrenScript>> ScriptingEngine::m_Scripts;
	std::map<std::string, Ref<ScriptModule>> ScriptingEngine::Modules;
	std::vector<std::string> ScriptingEngine::m_LoadedScripts;

	void errorFn(WrenVM* vm, WrenErrorType errorType,
		const char* moduleName, const int line,
		const char* msg)
	{
		switch (errorType)
		{
		case WREN_ERROR_COMPILE:
		{
			HZ_CORE_ERROR(std::string(moduleName) + " line " + std::to_string(line) + ": " + msg);
			Engine::ExitPlayMode();
		}
			break;
		case WREN_ERROR_STACK_TRACE:
		{
			HZ_CORE_ERROR("Stack trace: " + std::string(moduleName) + " line " + std::to_string(line) + ": " + msg);
		}
			break;
		case WREN_ERROR_RUNTIME:
		{
			HZ_CORE_WARN("Script Runtime Error: " + std::string(msg));
		}
			break;
		default:
		{
			HZ_CORE_ERROR("Script Runtime Error: " + std::string(msg));
		}
			break;
		}
	}

	void writeFn(WrenVM* vm, const char* text)
	{
		std::cout << text << "\n";
	}

	bool hasEnding(std::string const& fullString, std::string const& ending)
	{
		if (fullString.length() >= ending.length())
		{
			return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
		}
		else
		{
			return false;
		}
	}

#define GET_STATIC_RESOURCE_SCRIPT_SRC(name) std::string(reinterpret_cast<const char*>(name), reinterpret_cast<const char*>(name) + name##_len)

	std::map<std::string, std::string> _ModulesSRC =
	{
		//{ "Audio", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Audio_wren) },
		//{ "Engine", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Engine_wren) },
		//{ "Input", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Input_wren) },
		//{ "Math", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Math_wren) },
		//{ "Physics", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Physics_wren) },
		//{ "Scene", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_Scene_wren) },
		//{ "ScriptableEntity", GET_STATIC_RESOURCE_SCRIPT_SRC(StaticResources::Resources_Scripts_ScriptableEntity_wren) },
	};

	void onCompleteCB(WrenVM* vm, const char* name, WrenLoadModuleResult result)
	{
		delete result.source;
	}

	const std::string FaintModulePrefix = "Faint:";

	WrenLoadModuleResult myLoadModule(WrenVM* vm, const char* name)
	{
		WrenLoadModuleResult result = { 0 };

		result.onComplete = &onCompleteCB; // This will take care of the clean up of the new char*
		std::string sname = std::string(name);
		std::string path = "res/" + std::string(name);

		std::string fileContent = "";
		bool isAbsolute = false;
		if (sname.rfind(FaintModulePrefix, 0) == 0)
		{
			size_t prefixPosition = sname.find(FaintModulePrefix);
			sname.erase(prefixPosition, FaintModulePrefix.length());
			path = "res/Scripts/" + std::string(sname);

			if (_ModulesSRC.find(sname) != _ModulesSRC.end())
			{
				fileContent = _ModulesSRC[sname];
			}
			else
			{
				HZ_CORE_ERROR("Internal script module not found: " + sname + ". \n Did you forget to register it?");
			}
		}
		else
		{
			path = FileSystem::Root + name;
			isAbsolute = true;
		}

		if (!hasEnding(path, ".wren"))
			path += ".wren";

		if (isAbsolute)
		{
			fileContent = FileSystem::ReadFile(path, true);
		}

		// We have to use c style char array and the callback takes care of cleaning up.
		char* sourceC = new char[fileContent.length() + 1];
		char* c = strcpy(sourceC, fileContent.c_str()); // copy into c array

		result.source = c;
		return result;
	}

	bool ScriptingEngine::IsScriptImported(const std::string& path)
	{
		for (auto& script : m_LoadedScripts)
		{
			if (script == path)
				return true;
		}
		return false;
	}

	void ScriptingEngine::ImportScript(const std::string& path)
	{
		if (IsScriptImported(path))
			return;
		m_LoadedScripts.push_back(path);
	}

	void ScriptingEngine::RegisterModule(Ref<ScriptModule> scriptModule)
	{
		Modules[scriptModule->GetModuleName()] = scriptModule;
		scriptModule->RegisterModule(m_WrenVM);
	}

	void ScriptingEngine::InitScript(Ref<WrenScript> script)
	{
		script->CallInit();
	}

	void ScriptingEngine::UpdateScript(Ref<WrenScript> script, Time timestep)
	{
		script->CallUpdate(timestep);
	}

	void ScriptingEngine::ExitScript(Ref<WrenScript> script)
	{
		script->CallExit();
	}

	void ScriptingEngine::Init()
	{
		HZ_CORE_INFO("Initializing Scripting Engine *Wren");
		m_Scripts = std::map<std::string, Ref<WrenScript>>();
		m_LoadedScripts.clear();

		WrenConfiguration config;
		wrenInitConfiguration(&config);

		config.loadModuleFn = &myLoadModule;
		config.errorFn = &errorFn;
		config.writeFn = &writeFn;
		config.bindForeignMethodFn = &bindForeignMethod;
		m_WrenVM = wrenNewVM(&config);

		HZ_CORE_INFO("Registing Scripting Modules");
		//RegisterModule(CreateRef<)
		HZ_CORE_INFO("Scripting Modules Registered");
		HZ_CORE_INFO("Scripting Engine Initialized successfully");
	}

	void ScriptingEngine::Run(const std::string& code)
	{
	}

	void ScriptingEngine::Exit()
	{
		wrenFreeVM(m_WrenVM);
	}

	WrenForeignMethodFn ScriptingEngine::bindForeignMethod(WrenVM* vm, const char* modul, const char* className, bool isStatic, const char* signature)
	{
		if (Modules.find(className) != Modules.end())
		{
			Ref<ScriptModule> mod = Modules[className];
			void* ptr = mod->GetMethodPointer(signature);
			return (WrenForeignMethodFn)ptr;
		}

		return nullptr;
	}

	WrenVM* ScriptingEngine::GetWrenVM()
	{
		return m_WrenVM;
	}
}

