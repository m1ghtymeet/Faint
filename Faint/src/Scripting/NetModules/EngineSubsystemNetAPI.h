#pragma once
#include "NetAPIModule.h"

namespace Faint {

	class EngineSubsystemNetAPI : public Faint::NetAPIModule {
	public:
	
		const std::string GetModuleName() const override { return "EngineSubsystemNetAPI"; }
		void RegisterMethods() override;
	};
}