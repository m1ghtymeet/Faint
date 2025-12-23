#pragma 
#include "../UI/Panels/PanelWindow.h"

class Console : public PanelWindow {
public:
	Console(
		const std::string& p_name,
		bool p_opened,
		const PanelWindowSettings& p_windowSettings
	);

private:
	void _Draw_Impl() override;
};