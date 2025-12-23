#include "CSharpScriptEngine.h"
#include "CSharpScript.h"
#include <Debug/Log.h>
#include <Scene/Components/Behaviour.h>

void ExceptionCallback(std::string_view inMessage) {
	HZ_CORE_ERROR("[Coral] Unhandled native exception: {}", std::string(inMessage));
}

void MessageCallback(std::string_view message, Coral::MessageLevel level) {
	HZ_CORE_INFO("[Coral]: {}", std::string(message));
}

Moon::Scripting::CoralScriptEngine::CoralScriptEngine() {
	Coral::HostSettings settings = {
		.MessageCallback = MessageCallback,
		.ExceptionCallback = ExceptionCallback
	};
	settings.ExceptionCallback = [](std::string_view msg) {
		HZ_CORE_ERROR("[Coral] Unhandled Exception: {}", msg);
	};

	Coral::CoralInitStatus status = m_host.Initialize(settings);
	if (status != Coral::CoralInitStatus::Success)
		HZ_CORE_ERROR("[Coral] Failed to initialize Coral: {}", (int)status);

	// RegisterMethods

	m_context = m_host.CreateAssemblyLoadContext("MoonScriptContext");

	m_scriptDllPath = "MoonNet.dll";
	LoadScriptAssembly(m_scriptDllPath.string());
}

Moon::Scripting::CoralScriptEngine::~CoralScriptEngine() {
}

bool Moon::Scripting::CoralScriptEngine::LoadScriptAssembly(const std::string& dllPath) {
	try {
		m_assembly = &m_context.LoadAssembly(dllPath);
		HZ_CORE_INFO("[Coral] MoonNet.dll loaded successfully!");
		return true;
	}
	catch (const std::exception& ex) {
		HZ_CORE_ERROR("[Coral] Failed to load MoonNet.dll: {}", ex.what());
		return false;
	}
}

std::unique_ptr<Moon::Scripting::CoralScript> Moon::Scripting::CoralScriptEngine::CreateScript(const std::string& assemblyPath, const std::string& className) {
	auto script = std::make_unique<CoralScript>(assemblyPath, className);

	try {
		std::filesystem::path path(assemblyPath);
		if (!std::filesystem::exists(path)) {
			HZ_CORE_ERROR("[Coral] Assembly not found: {}", assemblyPath);
			return script;
		}
		auto type = m_assembly->GetType(className);
		if (type.GetSize() == 0) {
			HZ_CORE_ERROR("[Coral] Type not found: {}", className);
			return script;
		}

		script->m_instance = type.CreateInstance();
		script->m_isValid = true;

		HZ_CORE_INFO("[Coral] Script loaded: {} -> {}", assemblyPath, className);
	}
	catch (const std::exception& ex) {
		m_errorCount++;
	}
	return script;
}

void Moon::Scripting::CoralScriptEngine::OnUpdate(Behaviour& p_target, float p_deltaTime) {
	
}

void Moon::Scripting::CoralScriptEngine::HotloadIfNeeded() {
	if (!m_scriptDllPath.empty() && std::filesystem::exists(m_scriptDllPath)) {
		HZ_CORE_INFO("[Coral] Hot-reloading MoonNet.dll...");
		LoadScriptAssembly(m_scriptDllPath.string());
	}
}

std::string Moon::Scripting::CoralScriptEngine::GetDefaultScriptContent(const std::string& name) {
	return R"(using System;
using Moon;

public class )" + name + R"( : Behaviour
{
	public override void Start()
	{
		Console.WriteLine("Hello from Coral! Entity: " + Entity.name);
	}

	public override void Update(float dt)
	{
		transform.rotation += new Vector3(0, 5 * dt, 0);
	}
})";
}