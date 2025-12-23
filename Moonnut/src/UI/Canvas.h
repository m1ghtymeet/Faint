#pragma once

#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Panels/APanel.h"

class Canvas {
public:
	void AddPanel(APanel& p_panel);

	void RemovePanel(APanel& p_panel);

	void RemoveAllPanels();

	void MakeDockspace(bool p_state);

	bool IsDockspace() const;

	void Draw();

private:
	std::vector<std::reference_wrapper<APanel>> m_panels;
	bool m_isDockspace = false;
};