#pragma once
#include "Context.h"
#include "../EditorLayer.h"

class Application {
public:
	Application(const std::filesystem::path& m_projectFolder);

	virtual ~Application();

	void Run();

	bool IsRunning() const;

private:
	Context m_context;
	EditorLayer m_editor;
	float m_lastFrameTime = 0;
};