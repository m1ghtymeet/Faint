#pragma once
#include <string>
#include <map>

namespace Faint {

	class ScriptModule {
	public:
		std::string ModuleName;

		virtual void RegisterModule(WrenVM*) = 0;
		virtual std::string GetModuleName() = 0;

		void RegisterMethod(const std::string& signature, void* ptr)
		{
			Methods[signature] = ptr;
		}

		void* GetMethodPointer(const std::string& signature)
		{
			return Methods[signature];
		}

	private:
		std::map<std::string, void*> Methods = std::map<std::string, void*>();
		std::string WrenAPI;
	};
}