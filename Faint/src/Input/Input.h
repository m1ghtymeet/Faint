#pragma once

#include "Core/Base.h"

namespace Faint {

	class Input {
	public:
		static void Init();
		static void Update();
		static void ShowCursor();
		static void DisableCursor();
		static void HideCursor();

		static bool KeyDown(unsigned int keycode);
		static bool KeyPressed(unsigned int keycode);
		static float GetMouseXOffset();
		static float GetMouseYOffset();
		static bool LeftMouseDown();
		static bool RightMouseDown();
		static bool LeftMousePressed();
		static bool RightMousePressed();
		static bool MouseWheelUp();
		static bool MouseWheelDown();
		static int GetMouseX();
		static int GetMouseY();

	private:
		static bool _keyPressed[372];
		static bool _keyDown[372];
		static bool _keyDownLastFrame[372];
		static float _mouseX;
		static float _mouseY;
		static float _mouseXOffset;
		static float _mouseYOffset;
		static int _mouseWheelValue;
		static bool _mouseWheelUp;
		static bool _mouseWheelDown;
		static bool _leftMouseDown;
		static bool _rightMouseDown;
		static bool _leftMousePressed;
		static bool _rightMousePressed;
		static bool _leftMouseDownLastFrame;
		static bool _rightMouseDownLastFrame;
	};
}