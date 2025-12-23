#pragma once

#include <memory>

#include "APanel.h"
#include <glm/glm.hpp>

class APanelTransformable : public APanel {
public:
	APanelTransformable(
		const glm::vec2& p_defaultPosition = glm::vec2(-1.0f, -1.0f),
		const glm::vec2& p_defaultSize = glm::vec2(-1.0f, -1.0f),
		bool p_ignoreConfigFile = false
	);
	
	void SetSize(const glm::vec2& p_size);

	const glm::vec2& GetSize() const;

protected:
	void Update();
	virtual void _Draw_Impl() = 0;

private:
	void UpdateSize();

protected:
	glm::vec2 m_defaultPosition;
	glm::vec2 m_defaultSize;
	bool m_ignoreConfigFile;

	glm::vec2 m_position = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_size = glm::vec2(0.0f, 0.0f);

	bool m_positionChanged = false;
	bool m_sizeChanged = false;
};