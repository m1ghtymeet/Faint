#include "APanelTransformable.h"

#include <imgui.h>

APanelTransformable::APanelTransformable(const glm::vec2& p_defaultPosition, const glm::vec2& p_defaultSize, bool p_ignoreConfigFile) :
	m_defaultPosition(p_defaultPosition),
	m_defaultSize(p_defaultSize),
	m_ignoreConfigFile(p_ignoreConfigFile)
{
}

void APanelTransformable::SetSize(const glm::vec2& p_size) {
	m_size = p_size;
	m_sizeChanged = true;
}

const glm::vec2& APanelTransformable::GetSize() const {
	return m_size;
}

void APanelTransformable::Update() {
	
	UpdateSize();
	{
		m_size = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
	}
}

void APanelTransformable::UpdateSize() {
	if (m_sizeChanged) {
		ImGui::SetWindowSize({ m_size.x, m_size.y }, ImGuiCond_Always);
		m_sizeChanged = false;
	}
}
