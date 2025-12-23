#pragma once

#include <Event/Event.h>

#include "APanelTransformable.h"

/**
* A PanelWindow is a panel that is localized and behave like a window (Movable, resizable...)
*/
class PanelWindow : public APanelTransformable {
public:
	/**
	* Creates the PanelWindow
	* @param p_name
	* @param p_opened
	* @param p_panelSettings
	*/
	PanelWindow(
		const std::string& p_name = "",
		bool p_opened = true,
		const PanelWindowSettings& p_panelSettings = PanelWindowSettings{}
	);

	void Open();

	void Close();

	void Focus();

	void SetOpened(bool p_value);

	bool IsOpened() const;

protected:
	virtual void _Base_Draw() override;

public:
	std::string name;

	Moon::Event<> OpenEvent;
	Moon::Event<> CloseEvent;

private:
	bool m_opened = false;
	bool m_hovered = false;
	bool m_focused = false;
	bool closable = false;
};