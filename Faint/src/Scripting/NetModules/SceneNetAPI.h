#pragma once
#include "NetAPIModule.h"

namespace Faint {

	class SceneNetAPI : public NetAPIModule {
	public:

		virtual const std::string GetModuleName() const override { return "Scene"; };
		virtual void RegisterMethods() override;
	};
}