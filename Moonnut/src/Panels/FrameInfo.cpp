#include "FrameInfo.h"
#include <imgui.h>

namespace Moon::Editor {
	uint32_t m_fps = 0.0f;
}

Moon::Editor::FrameInfo::FrameInfo(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_title, p_opened, p_windowSettings) {
}

void Moon::Editor::FrameInfo::Update(std::shared_ptr<AView> p_targetView, float p_deltaTime) {
	m_framerateStats.elapsedFrames++;
	m_framerateStats.elapsedTime += p_deltaTime;

	if (m_framerateStats.elapsedTime >= (1.0f / 60.0f) * 10.0f) {
		float averageFramerate = m_framerateStats.elapsedFrames / m_framerateStats.elapsedTime;
		m_fps = static_cast<uint32_t>(std::roundf(averageFramerate));
		m_framerateStats = {};
	}
}

void Moon::Editor::FrameInfo::_Draw_Impl() {
	ImGui::Text("FPS: %d", m_fps);
	ImGui::Text("Frame Time: %0.3f ms", 1000.0f / m_fps);
}
