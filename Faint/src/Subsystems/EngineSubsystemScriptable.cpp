#include "EngineSubsystemScriptable.h"
#include "Scripting/ScriptingEngineNet.h"
#include <Coral/Type.hpp>

namespace Faint {

	EngineSubsystemScriptable::EngineSubsystemScriptable(const Coral::ManagedObject& object)
		: cSharpObjectInstance(object) {
	}

	EngineSubsystemScriptable::~EngineSubsystemScriptable()
	{
		if (!cSharpObjectInstance.IsValid())
			return;

		ScriptingEngineNet::Get().OnUninitialize().Remove(scriptEngineUninitializeDelegateHandle);

		cSharpObjectInstance.Destroy();
	}

	Coral::ManagedObject& EngineSubsystemScriptable::GetManagedObjectInstance()
	{
		// TODO: insert return statement here
		return cSharpObjectInstance;
	}

	void EngineSubsystemScriptable::Initialize()
	{
		scriptEngineUninitializeDelegateHandle = ScriptingEngineNet::Get().OnUninitialize().AddRaw(this, &EngineSubsystemScriptable::OnScriptEngineUninitialize);
	
		if (!cSharpObjectInstance.IsValid())
			return;

		cSharpObjectInstance.InvokeMethod("Initialize");
	}

	void Faint::EngineSubsystemScriptable::Update(float deltaTime)
	{
		if (!cSharpObjectInstance.IsValid())
			return;

		cSharpObjectInstance.InvokeMethod("Update", deltaTime);
	}

	void EngineSubsystemScriptable::OnScenePreInitialize(Ref<Scene> scene)
	{
	}

	void EngineSubsystemScriptable::OnScenePostInitialize(Ref<Scene> scene)
	{
	}

	void EngineSubsystemScriptable::OnScenePreDestroy(Ref<Scene> scene)
	{
	}

	void Faint::EngineSubsystemScriptable::OnScriptEngineUninitialize()
	{
		if (!cSharpObjectInstance.IsValid())
			return;

		cSharpObjectInstance.Destroy();
	}
}