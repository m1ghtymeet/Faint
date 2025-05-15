#include "EngineSubsystemNetAPI.h"

#include "Engine.h"
#include "Subsystems/EngineSubsystemScriptable.h"

namespace Faint {

	void SetCanTick(int subsystemId, bool tick) {
		
		auto subsystem = Engine::GetScriptedSubsystem(subsystemId);
		if (subsystem == nullptr) {
			HZ_CORE_ERROR("EngineSubsystemNet * Subsystem isn't a valid scripted subsystem");
			return;
		}

		subsystem->SetCanTick(tick);
	}

	bool GetCanTick(int subsystemId) {

		auto subsystem = Engine::GetScriptedSubsystem(subsystemId);
		if (subsystem == nullptr) {
			HZ_CORE_ERROR("EngineSubsystemNet * Subsystem isn't a valid scripted subsystem");
			return false;
		}

		return subsystem->CanEverTick();
	}

	void EngineSubsystemNetAPI::RegisterMethods()
	{
		RegisterMethod("EngineSubsystem.SetCanTickIcall", &SetCanTick);
		RegisterMethod("EngineSubsystem.GetCanTickIcall", &GetCanTick);
	}
}
