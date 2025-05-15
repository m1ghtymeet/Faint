#include "UISystem.h"

namespace Faint {

	UISystem::UISystem(Scene* scene)
	{
		m_scene = scene;
	}

	bool UISystem::Init()
	{
		return true;
	}

	void UISystem::Update(Time ts)
	{
	}

	void UISystem::FixedUpdate(Time ts)
	{
	}

	void UISystem::Exit()
	{
	}
}
