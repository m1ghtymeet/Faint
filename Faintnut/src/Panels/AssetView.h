#pragma once
#include "Panel.h"
#include <Debug/Log.h>
#include <string>

class Console {
public:
	/**
	* Constructor
	* @param p_title
	* @param p_opened
	* @param p_windowSettings
	*/
	Console(const std::string& p_title, bool p_opened, PanelWindowSettings p_windowSettings = PanelWindowSettings());
	void Draw();
};