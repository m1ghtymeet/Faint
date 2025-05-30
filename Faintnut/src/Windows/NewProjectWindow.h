#pragma once

#include "Core/Base.h"
#include "Core/Time.h"
#include "Core/Window.h"

class NewProjectWindow {
private:
	uint32_t m_SelectedTemplate = 0;
	bool m_HasCreatedProject = false;

public:
	NewProjectWindow() = default;

	void Draw();
	void DrawProject(int i, const std::string& title, const std::string& description);
	void DrawProjectTemplate(uint32_t i, const std::string& title, const std::string& description);

	bool HasCreatedProject() const {
		return m_HasCreatedProject;
	}
};