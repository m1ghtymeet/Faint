#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <Coral/GC.hpp>
#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/Assembly.hpp>
#include <Coral/HostInstance.hpp>

namespace Moon { class Behaviour; }
namespace Moon::Scripting {
	class CoralScript;
	class CoralScriptEngine {
	private:
		Coral::HostInstance m_host;
		Coral::AssemblyLoadContext m_context;
		Coral::ManagedAssembly* m_assembly = nullptr;

		std::filesystem::path m_scriptDllPath;
		std::vector<std::reference_wrapper<Behaviour>> m_behaviours;
		uint32_t m_errorCount = 0;

		static int ExceptionHandler(void* ex);
		void SetupCoral();
		void BindEngineTypes();
		bool LoadAssembly(const std::string& path);

	public:
		CoralScriptEngine();
		~CoralScriptEngine();
		CoralScriptEngine(const CoralScriptEngine&) = delete;
		CoralScriptEngine& operator=(const CoralScriptEngine&) = delete;

		bool LoadScriptAssembly(const std::string& dllPath);

		std::unique_ptr<CoralScript> CreateScript(const std::string& assemblyPath, const std::string& className);

		// Callbacks
		void OnAwake(Behaviour& p_target);
		void OnStart(Behaviour& p_target);
		void OnEnable(Behaviour& p_target);
		void OnUpdate(Behaviour& p_target, float p_deltaTime);
		//void OnTriggerEnter(Behaviour& p_target, ColliderComponent& otherBody);
		//void OnTriggerExit(Behaviour& p_target, ColliderComponent& otherBody);

		void ReloadAll();
		void HotloadIfNeeded();

		std::string GetDefaultScriptContent(const std::string& name);
	};
}