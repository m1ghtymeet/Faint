#pragma once
#include "../UI/Panels/PanelWindow.h"

namespace Moon::Editor {
	class ProjectSettingsPanel : public PanelWindow {
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		ProjectSettingsPanel(
			const std::string& p_title,
			bool p_opened,
			const PanelWindowSettings& p_windowSettings
		);
	private:
		void _Draw_Impl() override;
	};
}