#include "PanelMenuBar.h"

#include <imgui.h>

void PanelMenuBar::_Base_Draw() {

	if (ImGui::BeginMainMenuBar()) {
		DrawWidgets();
		ImGui::EndMainMenuBar();
	}
}

void PanelMenuBar::_Draw_Impl() {
}
