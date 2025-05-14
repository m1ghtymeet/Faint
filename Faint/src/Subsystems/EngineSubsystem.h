#pragma once

#include "Core/Base.h"

namespace Faint {

	class Scene;

	class EngineSubsystem {
	public:
		void SetCanTick(bool canTick);
		bool CanEverTick() const;

		virtual void Initialize() {}
		virtual void Update(float deltaTime) {}

		virtual void OnScenePreInitialize(Ref<Scene> scene) {}
		virtual void OnScenePostInitialize(Ref<Scene> scene) {}
		virtual void OnScenePreDestroy(Ref<Scene> scene) {}

	protected:
		void OnScriptEngineUninitialize();

		bool canEverTick = false;
	};
}