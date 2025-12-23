#pragma once

#include "../UI/Panels/PanelWindow.h"
#include "../UI/Panels/AView.h"

namespace Moon::Editor {
	struct FramerateStats {
		uint32_t elapsedFrames = 0;
		float elapsedTime = 0.0f;
	};
	class FrameInfo : public PanelWindow {
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		FrameInfo(
			const std::string& p_title,
			bool p_opened,
			const PanelWindowSettings& p_windowSettings
		);

		void Update(std::shared_ptr<AView> p_targetView, float p_deltaTime);

	private:
		void _Draw_Impl() override;
		FramerateStats m_framerateStats;
	};
}