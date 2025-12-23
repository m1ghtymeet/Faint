#pragma once

#include "../UI/Panels/PanelWindow.h"

class HardwareInfo : public PanelWindow {
public:
	HardwareInfo(
		const std::string& p_name,
		bool p_opened,
		const PanelWindowSettings& p_windowSettings
	);

private:
	void _Draw_Impl() override;
};