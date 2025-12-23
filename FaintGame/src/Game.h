#pragma once

#include "Context.h"
#include <Renderer/SceneRenderer.h>

class Game {
public:
	Game(Context& p_context);

	~Game();

	void Update(float p_deltaTime);

private:
	Context& m_context;
	Moon::Rendering::SceneRenderer* m_sceneRenderer = nullptr;
};