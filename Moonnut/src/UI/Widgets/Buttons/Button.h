#pragma once
#include "AButton.h"
#include <glm/glm.hpp>

class Button : public AButton {
public:
	Button(const std::string& p_label = "", const glm::vec2& p_size = { 0.0f, 0.0f }, bool p_disabled = false);

protected:
	void _Draw_Impl() override;

public:
	std::string m_label;
	glm::vec2 m_size;
	bool m_disabled = false;

	glm::vec4 textColor;
	glm::vec4 idleBackgroundColor;
	glm::vec4 hoveredBackgroundColor;
	glm::vec4 clickedBackgroundColor;
};