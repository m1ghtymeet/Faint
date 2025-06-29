#pragma once
#include <algorithm>
#include <string>

/**
* Data structure to send to the panel window constructor to define its settings
*/
struct PanelWindowSettings {
	PanelWindowSettings() = default;
	bool closable = false;
	bool resizable = false;
	bool movable = false;
	bool dockable = false;
	bool scrollable = false;
	bool hideBackground = false;
	bool forceHorizontalScrollbar = false;
	bool forceVerticalScrollbar = false;
	bool allowHorizontalScrollbar = false;
	bool bringToFrontOnFocus = false;
	bool collapsable = false;
	bool allowInputs = false;
	bool titleBar = false;
	bool autoSize = false;
};

class APanel {
public:
	APanel();

	void Draw();
	const std::string& GetPanelID() const;

	bool enabled = true;

protected:
	virtual void _Draw_Impl() = 0;
	std::string m_panelID;

private:
	static uint64_t __PANEL_ID_INCREMENT;
};