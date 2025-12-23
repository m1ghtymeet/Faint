#pragma once

#include "APanel.h"

class PanelMenuBar : public APanel {
private:
	virtual void DrawWidgets() = 0;
protected:
	void _Base_Draw() override;
	virtual void _Draw_Impl() override;
};