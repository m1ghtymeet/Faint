#pragma once

#include <optional>
#include <unordered_map>

#include "Settings/WindowSettings.h"

struct GLFWwindow;

namespace Moon {

	class Window {
	public:
		Window(const Settings::WindowSettings& p_settings);
		
		~Window();

		Window* FindInstance(GLFWwindow* p_glfwWindow);

		void SetSize(uint16_t p_width, uint16_t p_height);

		std::pair<uint16_t, uint16_t> GetSize() const;

		void SetClose(bool close);

		bool ShouldClose();
		
		void Destroy();

		bool HasFocus();

		// TODO: Remove
		void EndDraw();

		void SetPosition(int16_t p_x, int16_t p_);
		void SetTitle(const std::string& title);
		void SetIcon(const std::string& path);
		void SetDecorated(bool enabled);
		void SetFullscreen(bool enabled);
		void Maximize();
		void ShowWindow(bool show);

		void MakeCurrentContext() const;

		void SwapBuffers();

		GLFWwindow* GetGLFWWindow() const;

		void InitImgui();

	private:
		void CreateGLFWWindow(const Settings::WindowSettings& p_settings);

	private:
		static std::unordered_map<GLFWwindow*, Window*> __WINDOWS_MAP;

		GLFWwindow* m_windowPointer;
		Settings::WindowSettings m_windowSettings;

		/* Window settings */
		std::string m_title;
		std::pair<uint16_t, uint16_t> m_size;
		std::pair<int16_t, int16_t> m_position;
		bool m_fullscreen;
		int32_t m_refreshRate;
	};
}