#pragma once

#include "Context.h"
#include "Game.h"

class Application {
public:
	Application();

	~Application();

	void Run();

	bool IsRunning() const;

private:
	Context m_context;
	Game m_game;
};