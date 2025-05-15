#pragma once
#include "NetAPIModule.h"

namespace Faint {

	class EngineNetAPI : public NetAPIModule {
	public:
		virtual const std::string GetModuleName() const override { return "Engine"; }

		virtual void RegisterMethods() override;
	};
}