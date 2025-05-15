#include "Input.h"

#include "Engine.h"

#include <GLFW/glfw3.h>

namespace Faint {
	
	bool Input::_keyPressed[372];
	bool Input::_keyDown[372];
	bool Input::_keyDownLastFrame[372];
	float Input::_mouseX = 0;
	float Input::_mouseY = 0;
	float Input::_mouseXOffset = 0;
	float Input::_mouseYOffset = 0;
	int Input::_mouseWheelValue = 0;
	bool Input::_mouseWheelUp = false;
	bool Input::_mouseWheelDown = false;
	bool Input::_leftMouseDown = false;
	bool Input::_rightMouseDown = false;
	bool Input::_leftMousePressed = false;
	bool Input::_rightMousePressed = false;
	bool Input::_leftMouseDownLastFrame = false;
	bool Input::_rightMouseDownLastFrame = false;

	int _scrollWheelYOffset = 0;

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		_scrollWheelYOffset = (int)yoffset;
	}

	int GetScrollWheelYOffset() {
		return _scrollWheelYOffset;
	}

	void ResetScrollWheelYOffset() {
		_scrollWheelYOffset = 0;
	}

	void Input::Init()
	{
		double xpos, ypos;
		glfwSetScrollCallback(Engine::GetCurrentWindow()->GetWindow(), scroll_callback);
		glfwGetCursorPos(Engine::GetCurrentWindow()->GetWindow(), &xpos, &ypos);
		_mouseXOffset = xpos;
		_mouseXOffset = ypos;
		_mouseX = (float)xpos;
		_mouseY = (float)ypos;
	}

    void Input::Update() {

        for (int i = 32; i < 349; i++) {
            // down
            if (glfwGetKey(Engine::GetCurrentWindow()->GetWindow(), i) == GLFW_PRESS)
                _keyDown[i] = true;
            else
                _keyDown[i] = false;

            // pressed
            if (_keyDown[i] && !_keyDownLastFrame[i])
                _keyPressed[i] = true;
            else
                _keyPressed[i] = false;
            _keyDownLastFrame[i] = _keyDown[i];
        }

        // Mouse
        double xpos, ypos;
        glfwGetCursorPos(Engine::GetCurrentWindow()->GetWindow(), &xpos, &ypos);
        _mouseXOffset = (float)xpos - _mouseX;
        _mouseYOffset = (float)ypos - _mouseY;
        _mouseX = (float)xpos;
        _mouseY = (float)ypos;

        // Wheel
        _mouseWheelUp = false;
        _mouseWheelDown = false;
        _mouseWheelValue = GetScrollWheelYOffset();
        if (_mouseWheelValue < 0)
            _mouseWheelDown = true;
        if (_mouseWheelValue > 0)
            _mouseWheelUp = true;
        ResetScrollWheelYOffset();

        // Left mouse down/pressed
        _leftMouseDown = glfwGetMouseButton(Engine::GetCurrentWindow()->GetWindow(), GLFW_MOUSE_BUTTON_LEFT);
        if (_leftMouseDown == GLFW_PRESS && !_leftMouseDownLastFrame)
            _leftMousePressed = true;
        else
            _leftMousePressed = false;
        _leftMouseDownLastFrame = _leftMouseDown;

        // Right mouse down/pressed
        _rightMouseDown = glfwGetMouseButton(Engine::GetCurrentWindow()->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT);
        if (_rightMouseDown == GLFW_PRESS && !_rightMouseDownLastFrame)
            _rightMousePressed = true;
        else
            _rightMousePressed = false;
        _rightMouseDownLastFrame = _rightMouseDown;
    }

    void Input::ShowCursor() {
        glfwSetInputMode(Engine::GetCurrentWindow()->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Input::DisableCursor() {
        glfwSetInputMode(Engine::GetCurrentWindow()->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Input::HideCursor() {
        glfwSetInputMode(Engine::GetCurrentWindow()->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    bool Input::KeyDown(unsigned int keycode) {
        return _keyDown[keycode];
    }

    bool Input::KeyPressed(unsigned int keycode) {
        return _keyPressed[keycode];
    }

    float Input::GetMouseXOffset() {
        return _mouseXOffset;
    }

    float Input::GetMouseYOffset() {
        return _mouseYOffset;
    }

    bool Input::LeftMouseDown() {
        return _leftMouseDown;
    }

    bool Input::RightMouseDown() {
        return _rightMouseDown /* && !_preventRightMouseHoldTillNextClick*/;
    }

    bool Input::LeftMousePressed() {
        return _leftMousePressed;
    }

    bool Input::RightMousePressed() {
        return _rightMousePressed;
    }

    bool Input::MouseWheelUp() {
        return _mouseWheelUp;
    }

    bool Input::MouseWheelDown() {
        return _mouseWheelDown;
    }

    int Input::GetMouseX() {
        return _mouseX;
    }

    int Input::GetMouseY() {
        return _mouseY;
    }

    /*int Input::GetViewportMappedMouseX(int viewportWidth) {
        return Util::MapRange(GetMouseX(), 0, WindowManager::GetCurrentWindowWidth(), 0, viewportWidth);
    }

    int Input::GetViewportMappedMouseY(int viewportHeight) {
        return Util::MapRange(GetMouseY(), 0, WindowManager::GetCurrentWindowHeight(), 0, viewportHeight);
    }
    */
}
