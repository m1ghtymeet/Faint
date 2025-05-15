#include "WrenScript.h"
#include "FileSystem/File.h"

#include "Core/String.h"

namespace Faint {

	WrenScript::WrenScript(Ref<File> file, bool isEntityScript)
	{
		mFile = file;
		m_IsEntityScript = isEntityScript;

		ParseModules();
	}

	void WrenScript::ParseModules()
	{
		std::ifstream myReadFile(mFile->GetAbsolutePath());
		std::string fileContent = "";

		while (getline(myReadFile, fileContent))
		{
			bool hasFoundModule = false;

			auto splits = String::Split(fileContent, ' ');
			for (unsigned int i = 0; i < splits.size(); i++)
			{
				std::string s = splits[i];
				if (s == "class" && i + 1 < splits.size() && !hasFoundModule)
				{
					std::string moduleFound = splits[i + 1];
					m_Modules.push_back(moduleFound);
					hasFoundModule = true;
				}
			}
		}

		myReadFile.close();
	}

	void WrenScript::Build(unsigned int moduleId, bool isEntity) {

		WrenVM* vm = ScriptingEngine::GetWrenVM();

		const std::string& relativePath = mFile->GetRelativePath();

		// Can't import twice the same script, otherwise gives a compile error.
		if (!ScriptingEngine::IsScriptImported(relativePath))
		{
			const std::string& source = "import \"" + relativePath + "\" for " + GetModules()[moduleId];
			WrenInterpretResult result = wrenInterpret(vm, "main", source.c_str());

			if (result == WREN_RESULT_SUCCESS)
			{
				HZ_CORE_INFO("[ScriptingEngine] Compiled succesfully: " + std::to_string(result));
				m_HasCompiledSuccessfully = true;
				ScriptingEngine::ImportScript(relativePath);
			}
			else
			{
				HZ_CORE_ERROR("[ScriptingEngine] Compiled failed: " + std::to_string(result));
				m_HasCompiledSuccessfully = false;
				return;
			}
		}

		// Get handle to class
		wrenEnsureSlots(vm, 1);
		wrenGetVariable(vm, "main", GetModules()[moduleId].c_str(), 0);
		WrenHandle* classHandle = wrenGetSlotHandle(vm, 0);

		// Call the constructor
		WrenHandle* constructHandle = wrenMakeCallHandle(vm, "new()");
		wrenCall(vm, constructHandle);

		// Retreive value of constructor
		this->m_Instance = wrenGetSlotHandle(vm, 0);

		// Create handles to the instance methods.
		this->m_OnInitHandle = wrenMakeCallHandle(vm, "start()");
		this->m_OnUpdateHandle = wrenMakeCallHandle(vm, "update(_)");
		this->m_OnFixedUpdateHandle = wrenMakeCallHandle(vm, "fixedUpdate(_)");
		this->m_OnExitHandle = wrenMakeCallHandle(vm, "exit()");

		if (isEntity)
			this->m_SetEntityIDHandle = wrenMakeCallHandle(vm, "SetEntityId(_)");

		m_HasCompiledSuccessfully = true;
	}

	void WrenScript::CallInit()
	{
		if (!m_HasCompiledSuccessfully) return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		WrenInterpretResult result = wrenCall(vm, this->m_OnInitHandle);
	}

	void WrenScript::CallUpdate(float timestep)
	{
		
	}

	void WrenScript::CallFixedUpdate(float timestep)
	{
	}

	void WrenScript::CallExit()
	{
	}

	void WrenScript::SetScriptableEntityID(int id)
	{
		if (!m_HasCompiledSuccessfully)
			return;

		WrenVM* vm = ScriptingEngine::GetWrenVM();
		wrenSetSlotHandle(vm, 0, this->m_Instance);
		wrenSetSlotDouble(vm, 1, id);
		WrenInterpretResult result = wrenCall(vm, this->m_SetEntityIDHandle);
	}

	std::vector<std::string> WrenScript::GetModules() const
	{
		return m_Modules;
	}
}
