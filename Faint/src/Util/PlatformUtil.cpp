#include "PlatformUtil.h"
#include <Windows.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Engine.h"

#undef MessageBox

namespace Moon {

	HWND g_hProgressWnd = nullptr;
	HWND g_hProgressBar = nullptr;

	std::string FileDialogs::OpenFile(const char* filter) {

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		// Intiliaze OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = /*glfwGetWin32Window(Engine::GetCurrentWindow()->GetGLFWWindow())*/NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter) {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = /*glfwGetWin32Window(Engine::GetCurrentWindow()->GetGLFWWindow())*/NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::string();
	}

	MessageBox::MessageBox(std::string p_title, std::string p_message, EMessageType p_messageType, EButtonLayout p_buttonLayout, bool p_autoSpawn) :
		m_title(p_title),
		m_message(p_message),
		m_buttonLayout(p_buttonLayout),
		m_messageType(p_messageType) {
		if (p_autoSpawn)
			Spawn();
	}

	void MessageBox::Spawn() {
		int msgboxID = MessageBoxA
		(
			nullptr,
			static_cast<LPCSTR>(m_message.c_str()),
			static_cast<LPCSTR>(m_title.c_str()),
			static_cast<UINT>(m_messageType) | static_cast<UINT>(m_buttonLayout) | MB_DEFBUTTON2
		);

		m_userResult = static_cast<EUserAction>(msgboxID);
	}
}