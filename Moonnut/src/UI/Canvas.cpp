#include "Canvas.h"

#include <imgui_internal.h>

void Canvas::AddPanel(APanel& p_panel) {
	m_panels.push_back(std::ref(p_panel));
}

void Canvas::RemovePanel(APanel& p_panel) {
	m_panels.erase(std::remove_if(m_panels.begin(), m_panels.end(), [&p_panel](std::reference_wrapper<APanel>& p_item) {
		return &p_panel == &p_item.get();
	}));
}

void Canvas::RemoveAllPanels() {
	m_panels.clear();
}

void Canvas::MakeDockspace(bool p_state) {
	m_isDockspace = p_state;
}

bool Canvas::IsDockspace() const {
	return m_isDockspace;
}

void Canvas::Draw() {

	//if (!m_panels.empty()) {

		if (m_isDockspace) {
			static bool dockSpaceOpen = true;
			static bool opt_fullscreen = true;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen) {
			    ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
			}
			else {
			    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("##dockspace", &dockSpaceOpen, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// DockSpace
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			float minWinSize = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			ImGui::End();
		}

		for (auto& panel : m_panels)
			panel.get().Draw();

		//ImGui::Render();
	//}
}
