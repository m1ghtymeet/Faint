#include "AWidget.h"
#include <imgui.h>

uint64_t AWidget::__WIDGET_ID_INCREMENT = 0;

AWidget::AWidget() {
	m_widgetID = "##" + std::to_string(__WIDGET_ID_INCREMENT++);
}

void AWidget::Draw() {
	if (enabled) {
		if (disabled)
			ImGui::BeginDisabled();

		_Draw_Impl();

		if (!tooltip.empty()) {
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip(tooltip.c_str());
		}

		if (disabled)
			ImGui::EndDisabled();

		if (!lineBreak)
			ImGui::SameLine();
	}
}

void AWidget::Destroy() {

}

bool AWidget::IsDestroyed() const {
	return false;
}
