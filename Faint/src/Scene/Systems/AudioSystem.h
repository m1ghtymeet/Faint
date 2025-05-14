#pragma once
#include "Scene/Systems/System.h"

namespace Faint {

	class AudioSystem : public System {
	private:
		std::unordered_map<uint32_t, bool> m_StatusCache;

	public:
		AudioSystem(Scene* scene);
		bool Init() override;
		void Update(Time ts) override;
		void Draw() override {}
		void FixedUpdate(Time ts) override;
		void Exit() override;
	};
}