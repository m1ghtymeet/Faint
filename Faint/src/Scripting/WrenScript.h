#pragma once
#include "ScriptingEngine.h"

struct WrenHandle;

namespace Faint {

	class File;

	class WrenScript {
	private:
		bool m_HasCompiledSuccessfully;
		bool m_IsEntityScript = false;
		std::vector<std::string> m_Modules;

		Ref<File> mFile;

		std::map<std::string, WrenHandle*> methods;
		WrenHandle* m_Instance = nullptr;
		WrenHandle* m_OnInitHandle;
		WrenHandle* m_OnUpdateHandle;
		WrenHandle* m_OnFixedUpdateHandle;
		WrenHandle* m_OnExitHandle;
		WrenHandle* m_SetEntityIDHandle;

	public:
		WrenScript(Ref<File> file, bool isEntityScript);
		void ParseModules();

		void Build(unsigned int moduleId, bool isEntityScript = false);

		// Method calls
		void CallInit();
		void CallUpdate(float timestep);
		void CallFixedUpdate(float timestep);
		void CallExit();

		void RegisterMethod(const std::string& signature);
		void CallMethod(const std::string& signature);
		Ref<File> GetFile() const;

		void SetScriptableEntityID(int id);
		bool HasCompiledSuccesfully() const { return m_HasCompiledSuccessfully; }

		std::vector<std::string> GetModules() const;
	};
}