#pragma once

#include <string>

namespace Moon::Settings {

	/**
	* Contains window settings
	*/
	struct WindowSettings {

		/**
		* A simple constant used to ignore a value setting (Let the program decide for you)
		* @note You can you WindowSettings::DontCare only where it is indicated
		*/
		static const int32_t DontCare = -1;

		/**
		* Title of the window (Displayed in the title bar)
		*/
		std::string title = "Window";

		uint16_t width = 1280;

		uint16_t height = 720;

		int16_t x = 0;

		int16_t y = 0;

		int16_t minimumWidth = DontCare;

		int16_t minimumHeight = DontCare;

		int16_t maximumWidth = DontCare;
				
		int16_t maximumHeight = DontCare;

		bool fullscreen = false;

		bool decorated = false;

		bool resizable = false;

		bool focused = true;

		bool maximized = false;

		bool visible = true;

		int32_t refreshRate = DontCare;
	};
}