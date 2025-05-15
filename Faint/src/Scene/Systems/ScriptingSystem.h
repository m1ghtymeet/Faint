#pragma once
#include "System.h"
#include "Debug/MulticastDelegate.h"

namespace Faint {

	class Scene;

	class ScriptingSystem : public System {
	public:
		ScriptingSystem(Scene* scene);
		bool Init() override;
		void Update(Time ts) override;
		void Draw() override {};
		void FixedUpdate(Time ts) override;
		void Exit() override;

		void InitializeNewScripts();
	private:
		void DispatchPhysicCallbacks();
	};
}