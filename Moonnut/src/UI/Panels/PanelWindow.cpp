#include "PanelWindow.h"
#include <imgui.h>
#include "PanelUndecorated.h"

PanelWindow::PanelWindow(const std::string& p_name, bool p_opened, const PanelWindowSettings& p_panelSettings) :
	name(p_name),
	m_opened(p_opened)
{
}

void PanelWindow::Open() {
	if (!m_opened) {
		m_opened = true;
		OpenEvent.Invoke();
	}
}

void PanelWindow::Close() {
	if (m_opened) {
		m_opened = false;
		CloseEvent.Invoke();
	}
}

void PanelWindow::Focus() {
	ImGui::SetWindowFocus((name).c_str());
}

void PanelWindow::SetOpened(bool p_value) {
	m_opened = p_value;
}

bool PanelWindow::IsOpened() const {
	return m_opened;
}

void PanelWindow::_Base_Draw() {
	if (m_opened) {
		int windowFlags = ImGuiWindowFlags_None;

		//if (!resizable)					windowFlags |= ImGuiWindowFlags_NoResize;
		//if (!movable)					windowFlags |= ImGuiWindowFlags_NoMove;
		//if (!dockable)					windowFlags |= ImGuiWindowFlags_NoDocking;
		//if (hideBackground)				windowFlags |= ImGuiWindowFlags_NoBackground;
		//if (forceHorizontalScrollbar)	windowFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
		//if (forceVerticalScrollbar)		windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
		//if (allowHorizontalScrollbar)	windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;
		//if (!bringToFrontOnFocus)		windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		//if (!collapsable)				windowFlags |= ImGuiWindowFlags_NoCollapse;
		//if (!allowInputs)				windowFlags |= ImGuiWindowFlags_NoInputs;
		//if (!scrollable)                windowFlags |= ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
		//if (!titleBar)					windowFlags |= ImGuiWindowFlags_NoTitleBar;

		if (ImGui::Begin(name.c_str(), closable ? &m_opened : nullptr, windowFlags)) {
			m_hovered = ImGui::IsWindowHovered();
			m_focused = ImGui::IsWindowFocused();

			if (!m_opened)
				CloseEvent.Invoke();

			Update();
			_Draw_Impl();
			DrawWidgets();
		}
		ImGui::End();
	}
}
