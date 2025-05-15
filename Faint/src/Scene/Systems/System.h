#pragma once

#include "Core/Time.h"
#include "Core/Base.h"
#include "Debug/MulticastDelegate.h"

namespace Faint {

	class Scene;
	class System {
	public:
		Scene* m_scene;

		virtual bool Init() = 0;

		virtual void Draw() = 0;

		virtual void Update(Time ts) = 0;
		virtual void FixedUpdate(Time ts) = 0;
		virtual void EditorUpdate() {};
		virtual void Exit() = 0;

		MulticastDelegate<>& OnPreInit() { return preInitDelegate; }
		MulticastDelegate<>& OnPostInit() { return postInitDelegate; }

	protected:
		MulticastDelegate<> preInitDelegate;
		MulticastDelegate<> postInitDelegate;
	};
}