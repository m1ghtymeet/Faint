#include "Window.h"
#include <Debug/Assertion.h>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <FontAwesome5.h>

#include <stb_image.h>

namespace Moon {

	GLenum glCheckError_(const char* file, int line) {
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			std::cout << error << " | " << file << " (" << line << ")\n";
		}
		return errorCode;
	}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei /*length*/, const char* message, const void* /*userParam*/) {
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
		std::cout << "---------------\n";
		std::cout << "Debug message (" << id << "): " << message << "\n";
		switch (source) {
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		}
		std::cout << "\n";
		switch (type) {
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		}
		std::cout << "\n";
		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		}    std::cout << "\n\n\n";
	}

	std::unordered_map<GLFWwindow*, Window*> Window::__WINDOWS_MAP;
	bool g_windowHasFocus = true;

	Window::Window(const Settings::WindowSettings& p_settings) :
		m_windowSettings(p_settings),
		m_title(p_settings.title),
		m_size(p_settings.width, p_settings.height),
		m_position(p_settings.x, p_settings.y),
		m_fullscreen(p_settings.fullscreen),
		m_refreshRate(p_settings.refreshRate) {
		CreateGLFWWindow(p_settings);
	}

	Window::~Window() {
		//glfwDestroyWindow(m_windowPointer);
	}

	Window* Window::FindInstance(GLFWwindow* p_glfwWindow) {
		return __WINDOWS_MAP.find(p_glfwWindow) != __WINDOWS_MAP.end() ? __WINDOWS_MAP[p_glfwWindow] : nullptr;
	}

	void Window::SetSize(uint16_t p_width, uint16_t p_height) {
		glfwSetWindowSize(m_windowPointer, p_width, p_height);
	}

	std::pair<uint16_t, uint16_t> Window::GetSize() const {
		int width, height;
		glfwGetWindowSize(m_windowPointer, &width, &height);
		return std::make_pair(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
	}

	void Window::SetClose(bool close) {
		glfwSetWindowShouldClose(m_windowPointer, close);
	}

	void GLFWErrorCallback(int error, const char* description) {
		HZ_CORE_ERROR("GLFW Error " + std::to_string(error) + ": " + description + "");
	}

	void Window::CreateGLFWWindow(const Settings::WindowSettings& p_settings) {

		if (!glfwInit())
			HZ_CORE_ERROR("GLFW Initialization Failed!");
		glfwSetErrorCallback(GLFWErrorCallback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

		/* ====== */
		GLFWmonitor* selectedMonitor = nullptr;

		if (m_fullscreen)
			selectedMonitor = glfwGetPrimaryMonitor();

		glfwWindowHint(GLFW_RESIZABLE, p_settings.resizable);
		glfwWindowHint(GLFW_VISIBLE, p_settings.visible);

		m_windowPointer = glfwCreateWindow(static_cast<int>(m_size.first), static_cast<int>(m_size.second), m_title.c_str(), selectedMonitor, nullptr);

		if (!m_windowPointer) {
			FT_CORE_ASSERT(m_windowPointer != nullptr, "Failed to create GLFW window.");
		}
		else {
			//glfwSetWindowPos(m_windowPointer, static_cast<int>(m_position.first), static_cast<int>(m_position.second));
			__WINDOWS_MAP[m_windowPointer] = this;
		}

		MakeCurrentContext();

		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
			glDebugMessageCallback(glDebugOutput, nullptr);
		}
		else {
			std::cout << "Debug GL context not available\n";
		}

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_windowPointer, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		
		glfwSetWindowUserPointer(m_windowPointer, this);
	}

	bool Window::ShouldClose() {
		return !glfwWindowShouldClose(m_windowPointer);
	}

	void Window::Destroy() {
		glfwDestroyWindow(m_windowPointer);
	}

	bool Window::HasFocus() {
		return g_windowHasFocus;
	}

	void Window::EndDraw() {
		//ImGui::EndFrame();
		//ImGui::Render();
		//
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//
		//if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	GLFWwindow* backup_current_context = glfwGetCurrentContext();
		//	ImGui::UpdatePlatformWindows();
		//	ImGui::RenderPlatformWindowsDefault();
		//	glfwMakeContextCurrent(backup_current_context);
		//}

		glfwPollEvents();
	}

	void Window::SetPosition(int16_t p_x, int16_t p_y) {
		glfwSetWindowPos(m_windowPointer, static_cast<int>(p_x), static_cast<int>(p_y));
	}

	void Window::SetTitle(const std::string& title) {
		m_title = title;
		glfwSetWindowTitle(m_windowPointer, title.c_str());
	}

	void Window::SetIcon(const std::string& path) {
		GLFWimage images[1];
		stbi_set_flip_vertically_on_load(false);
		images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4);
		glfwSetWindowIcon(m_windowPointer, 1, images);
		stbi_image_free(images[0].pixels);
	}

	void Window::SetDecorated(bool enabled) {
		glfwSetWindowAttrib(m_windowPointer, GLFW_DECORATED, enabled);
	}

	void Window::SetFullscreen(bool enabled) {
		if (enabled)
			m_fullscreen = true;

		glfwSetWindowMonitor(
			m_windowPointer,
			enabled ? glfwGetPrimaryMonitor() : nullptr,
			static_cast<int>(m_position.first),
			static_cast<int>(m_position.second),
			static_cast<int>(m_size.first),
			static_cast<int>(m_size.second),
			m_refreshRate
		);

		if (!enabled)
			m_fullscreen = false;
	}

	void Window::Maximize() {
		const auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		//SetSize({ mode->width, mode->height });
		glfwMaximizeWindow(m_windowPointer);
	}

	void Window::ShowWindow(bool show) {
		glfwSetWindowAttrib(m_windowPointer, GLFW_VISIBLE, show);
	}

	void Window::MakeCurrentContext() const {
		glfwMakeContextCurrent(m_windowPointer);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			HZ_CORE_ERROR("Glad Initialization Failed!");
		}
	}

	void Window::SwapBuffers() {
		glfwPollEvents();

		int focused = glfwGetWindowAttrib(m_windowPointer, GLFW_FOCUSED);
		g_windowHasFocus = (focused != 0);

		glfwSwapBuffers(m_windowPointer);
	}

	GLFWwindow* Window::GetGLFWWindow() const {
		return m_windowPointer;
	}

	void Window::InitImgui()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		//ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		// io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		// io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig iconsConfigBold;

		float fontSize = 20.0f;
		io.Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Regular.ttf", fontSize);

		iconsConfigBold.MergeMode = true;
		ImFont* iconBoldFont = io.Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Regular.ttf", 17, &iconsConfigBold, icon_ranges);
		iconsConfigBold.MergeMode = false;

		io.FontDefault = iconBoldFont;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		ImVec4* colors = style.Colors;

		// تنظیمات گردی گوشه‌ها
		style.WindowRounding = 6.0f;
		style.ChildRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 6.0f;
		style.ScrollbarRounding = 6.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;

		// فاصله‌ها و padding
		style.WindowPadding = ImVec2(8, 8);
		style.FramePadding = ImVec2(8, 4);
		style.ItemSpacing = ImVec2(8, 4);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 20.0f;
		style.ScrollbarSize = 12.0f;
		style.GrabMinSize = 10.0f;

		// رنگ پس‌زمینه‌ها
		colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);

		// متن
		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.52f, 1.00f);

		// حاشیه‌ها
		colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// عنوان پنجره
		colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.08f, 0.75f);

		//ImVec4 accent = ImVec4(0.00f, 0.90f, 0.55f, 1.00f);
		//ImVec4 accentHover = ImVec4(0.00f, 0.95f, 0.60f, 1.00f);
		//ImVec4 accentActive = ImVec4(0.00f, 0.80f, 0.50f, 1.00f);

		// آکسانت ملایم (سبز کمرنگ)
		ImVec4 accent = ImVec4(0.30f, 0.70f, 0.50f, 1.00f); // سبز ملایم
		ImVec4 accentHover = ImVec4(0.35f, 0.75f, 0.55f, 1.00f);
		ImVec4 accentActive = ImVec4(0.25f, 0.65f, 0.45f, 1.00f);

		// دکمه‌ها
		colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = accentHover;
		colors[ImGuiCol_ButtonActive] = accentActive;

		// دکمه آکسانت
		colors[ImGuiCol_Button] = accent;
		colors[ImGuiCol_ButtonHovered] = accentHover;
		colors[ImGuiCol_ButtonActive] = accentActive;

		// فریم‌ها (ورودی‌ها)
		colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);

		// اسلایدرها
		colors[ImGuiCol_SliderGrab] = accent;
		colors[ImGuiCol_SliderGrabActive] = accentActive;

		// تب‌ها
		colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(accent.x, accent.y, accent.z, 0.25f);
		colors[ImGuiCol_TabActive] = accent;
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);

		// هدرها (برای لیست‌ها)
		colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.25f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
		colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.55f);

		// کامبو باکس
		colors[ImGuiCol_CheckMark] = accent;

		// اسکرول بار
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);

		// جداکننده‌ها
		colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);

		// فعال‌سازی viewport
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Initialize platform/renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_windowPointer, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}
}
