#include "EngineSubsystem.h"
#include "EngineSubsystem.h"

namespace Faint {
	void EngineSubsystem::SetCanTick(bool canTick) {
		canEverTick = canTick;
	}

	bool EngineSubsystem::CanEverTick() const {
		return canEverTick;
	}

	void Faint::EngineSubsystem::OnScriptEngineUninitialize()
	{
	}
}