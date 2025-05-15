#pragma once
#include "NetAPIModule.h"

namespace Faint {

	class InputNetAPI : public NetAPIModule {
	public:

		virtual const std::string GetModuleName() const override { return "Input"; };
		virtual void RegisterMethods() override;
	};
}