#include "InputNetAPI.h"

#include "Math/Math.h"
#include "Input/Input.h"

#include <Coral/Array.hpp>
#include <Coral/String.hpp>

namespace Faint {

	void ShowCursor(bool visible) {
		if (visible) {
			Input::ShowCursor();
		}
		else {
			Input::HideCursor();
		}
	}

	void DisableCursor() {
		Input::DisableCursor();
	}

	bool LeftMouseDown() {
		return Input::LeftMouseDown();
	}

	bool RightMouseDown() {
		return Input::RightMouseDown();
	}

	bool LeftMousePressed() {
		return Input::LeftMousePressed();
	}

	bool RightMousePressed() {
		return Input::RightMousePressed();
	}

	bool KeyDown(int keyCode) {
		return Input::KeyDown(keyCode);
	}

	bool KeyPressed(int keyCode) {
		return Input::KeyPressed(keyCode);
	}

	Coral::Array<float> GetMousePosition() {
		Vec2 mousePosition = Vec2(Input::GetMouseX(), Input::GetMouseY());
		return Coral::Array<float>::New({ mousePosition.x, mousePosition.y });
	}

	Coral::Array<float> GetMousePositionOffset() {
		Vec2 mousePosition = Vec2(Input::GetMouseXOffset(), Input::GetMouseYOffset());
		return Coral::Array<float>::New({ mousePosition.x, mousePosition.y });
	}

	void InputNetAPI::RegisterMethods()
	{
		RegisterMethod("Input.ShowMouseIcall", &ShowCursor);
		RegisterMethod("Input.DisableMouseIcall", &DisableCursor);
		RegisterMethod("Input.KeyDownIcall", &KeyDown);
		RegisterMethod("Input.KeyPressedIcall", &KeyPressed);

		RegisterMethod("Input.LeftMouseDownIcall", &LeftMouseDown);
		RegisterMethod("Input.RightMouseDownIcall", &RightMouseDown);
		RegisterMethod("Input.LeftMousePressedIcall", &LeftMousePressed);
		RegisterMethod("Input.RightMousePressedIcall", &RightMousePressed);
		RegisterMethod("Input.GetMousePositionIcall", &GetMousePosition);
		RegisterMethod("Input.GetMousePositionOffsetIcall", &GetMousePositionOffset);
	}
}
