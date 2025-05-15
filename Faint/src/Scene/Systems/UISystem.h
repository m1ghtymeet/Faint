#pragma once
#include "Scene/Systems/System.h"
#include "Common/UUID.h"

namespace Faint {

	class UISystem : public System {
	public:
		UISystem(Scene* scene);
		bool Init() override;
		void Update(Time ts) override;
		void Draw() override {}
		void FixedUpdate(Time ts) override;
		void Exit() override;

	private:
		//std::map<UUID
	};
}