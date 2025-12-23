#include "Button.h"
#include <imgui.h>

Button::Button(const std::string& p_label, const glm::vec2& p_size, bool p_disabled)
	: m_label(p_label), m_size(p_size), m_disabled(p_disabled) {

	auto& style = ImGui::GetStyle();  
	
	idleBackgroundColor = { style.Colors[ImGuiCol_Button].x, style.Colors[ImGuiCol_Button].y, style.Colors[ImGuiCol_Button].z, style.Colors[ImGuiCol_Button].w };  
	hoveredBackgroundColor = { style.Colors[ImGuiCol_ButtonHovered].x, style.Colors[ImGuiCol_ButtonHovered].y, style.Colors[ImGuiCol_ButtonHovered].z, style.Colors[ImGuiCol_ButtonHovered].w };  
	clickedBackgroundColor = { style.Colors[ImGuiCol_ButtonActive].x, style.Colors[ImGuiCol_ButtonActive].y, style.Colors[ImGuiCol_ButtonActive].z, style.Colors[ImGuiCol_ButtonActive].w };  
	textColor = { style.Colors[ImGuiCol_Text].x, style.Colors[ImGuiCol_Text].y, style.Colors[ImGuiCol_Text].z, style.Colors[ImGuiCol_Text].w };  
}

void Button::_Draw_Impl() {
	auto& style = ImGui::GetStyle();

	auto defaultIdleColor = style.Colors[ImGuiCol_Button];
	auto defaultHoveredColor = style.Colors[ImGuiCol_ButtonHovered];
	auto defaultClickedColor = style.Colors[ImGuiCol_ButtonActive];
	auto defaultTextColor = style.Colors[ImGuiCol_Text];

	style.Colors[ImGuiCol_Button] = { idleBackgroundColor.x, idleBackgroundColor.y, idleBackgroundColor.z, idleBackgroundColor.w };
	style.Colors[ImGuiCol_ButtonHovered] = { hoveredBackgroundColor.x, hoveredBackgroundColor.y, hoveredBackgroundColor.z, hoveredBackgroundColor.w };
	style.Colors[ImGuiCol_ButtonActive] = { clickedBackgroundColor.x, clickedBackgroundColor.y, clickedBackgroundColor.z, clickedBackgroundColor.w };
	style.Colors[ImGuiCol_Text] = { textColor.x, textColor.y, textColor.z, textColor.w };

	const bool isDisabled = m_disabled;

	if (isDisabled)
		ImGui::BeginDisabled();

	if (ImGui::Button((m_label + m_widgetID).c_str(), { m_size.x, m_size.y })) {
		ClickedEvent.Invoke();
	}

	if (isDisabled)
		ImGui::EndDisabled();

	style.Colors[ImGuiCol_Button] = defaultIdleColor;
	style.Colors[ImGuiCol_ButtonHovered] = defaultHoveredColor;
	style.Colors[ImGuiCol_ButtonActive] = defaultClickedColor;
	style.Colors[ImGuiCol_Text] = defaultTextColor;
}
