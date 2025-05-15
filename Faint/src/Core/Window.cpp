#include "Window.h"

#include "Math/Math.h"
#include "Scene/Scene.h"

#include "Engine.h"
#include "Renderer/Renderer.h"
#include "Renderer/Types/Framebuffer2.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "AssetManagment/Project.h"
#include "AssetManagment/FontAwesome5.h"

#include <stb_image.h>

namespace Faint {

	Ref<Window> Window::instance;

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

	Window::Window() :
		title("Hello Faint"),
		width(1280),
		height(720)
	{
		//instance = CreateRef<Window>();
		Init();
		Renderer::Init();
	}

	Ref<Window> Window::Get()
	{
		static Ref<Window> instance;
		if (instance == nullptr)
		{
			instance = CreateRef<Window>();
		}

		return instance;
	}

	GLFWwindow* Window::GetWindow()
	{
		return window;
	}

	bool Window::ShouldClose()
	{
		return !glfwWindowShouldClose(window);
	}

	static void GLFWErrorCallback(int error, const char* description) {
		HZ_CORE_ERROR("GLFW Error " + std::to_string(error) + ": " + description + "");
	}

	void Window::Init()
	{
		if (!glfwInit()) {
			HZ_CORE_ERROR("GLFW Initialization Failed!");
		}

		glfwSetErrorCallback(GLFWErrorCallback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		//ProjectSettings ps = Engine::GetProject()->Settings;
		//glfwWindowHint(GLFW_RESIZABLE, ps.resizable);

		window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		if (!window) {
			HZ_CORE_ERROR("Window Creation Failed!");
		}

		glfwMakeContextCurrent(window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			HZ_CORE_ERROR("Glad Initialization Failed!");
		}

		HZ_CORE_TRACE("Renderer: " + std::string((char*)glGetString(GL_RENDERER)));
		HZ_CORE_TRACE("Vendor: " + std::string((char*)glGetString(GL_VENDOR)));
		HZ_CORE_TRACE("Version: " + std::string((char*)glGetString(GL_VERSION)));

		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			//std::cout << "Debug GL context enabled\n";
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
			glDebugMessageCallback(glDebugOutput, nullptr);
		}
		else {
			std::cout << "Debug GL context not available\n";
		}

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		glfwSetWindowUserPointer(window, this);

		glfwSetDropCallback(window, [](GLFWwindow* newWindow, int count, const char** paths) {

			std::vector<std::string> filePaths;
			filePaths.reserve(count);

			int i;
			for (i = 0; i < count; i++) {
				std::string filePath = std::string(paths[i]);
				filePaths.push_back(filePath);
			}

			Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(newWindow));
			// Functions Addded?
		});

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		InitImgui();
	}

	void Window::Update(Time time)
	{
		scene->Update(time);
	}

	void Window::FixedUpdate(Time time)
	{
		scene->FixedUpdate(time);
	}

	void Window::Draw()
	{
		// ZoneScoped

		if (!scene)
			return;

		Ref<Camera> cam = scene->GetCurrentCamera();
		//if (!cam) return;

		if (Engine::IsPlayMode()) {
			scene->Draw();
		}
		else {
			float resolutionScale = glm::clamp(Engine::GetProject()->Settings.ResolutionScale, 0.5f, 2.0f);
			scene->m_EditorCamera->SetViewportSize(viewportWidth * resolutionScale, viewportHeight * resolutionScale);
			scene->Draw(scene->m_EditorCamera->GetProjectionMatrix(), scene->m_EditorCamera->GetViewMatrix());
		}

		//glEnable(GL_DEPTH_TEST);
		Renderer::EndScene();
	}

	void Window::EndDraw()
	{
		ImGui::EndFrame();
		ImGui::Render();
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	void Window::SetScene(Ref<Scene> scene)
	{
		this->scene = scene;
	}

	Ref<Scene> Window::GetScene()
	{
		return scene;
	}

	void Window::SetSize(const Vec2& size)
	{
	}

	Vec2 Window::GetSize() const
	{
		int w, h = 0;
		glfwGetWindowSize(window, &w, &h);
		return Vec2(w, h);
	}

	void Window::SetPosition(const Vec2& position)
	{
		//m_position = position;
		glfwSetWindowPos(window, static_cast<int>(position.x), static_cast<int>(position.y));
	}

	void Window::SetTitle(const std::string& title)
	{
		this->title = title;
		glfwSetWindowTitle(window, title.c_str());
	}

	void Window::SetIcon(const std::string& path)
	{
		GLFWimage images[1];
		images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4);
		glfwSetWindowIcon(window, 1, images);
		stbi_image_free(images[0].pixels);
	}

	void Window::SetDecorated(bool enabled)
	{
		glfwSetWindowAttrib(window, GLFW_DECORATED, enabled);
	}

	void Window::SetFullscreen(bool enabled)
	{
		const auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}

	void Window::Maximize()
	{
		const auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		SetSize({ mode->width, mode->height });
		glfwMaximizeWindow(window);
	}

	void Window::ShowWindow(bool show)
	{
		glfwSetWindowAttrib(window, GLFW_VISIBLE, show);
	}

	void Window::InitImgui()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		// io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		// io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig iconsConfigBold;

		float fontSize = 20.0f;
		io.Fonts->AddFontFromFileTTF("data/editor/fonts/Open-Sans-Bold.ttf", fontSize);

		iconsConfigBold.MergeMode = true;
		ImFont* iconBoldFont = io.Fonts->AddFontFromFileTTF("data/editor/fonts/fa-regular-400.ttf", fontSize, &iconsConfigBold, icon_ranges);
		iconsConfigBold.MergeMode = false;

		//io.FontDefault = io.Fonts->AddFontFromFileTTF("res/Fonts/FBBB.ttf", fontSize);
		io.FontDefault = iconBoldFont;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.078f, 0.078f, 0.078f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.93f, 0.27f, 0.27f, 0.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.08f, 0.49f, 0.97f, 0.28f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.55f, 0.76f, 0.29f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		ImGuiStyle& s = ImGui::GetStyle();
		//s.WindowMenuButtonPosition = ImGuiDir_None;
		s.GrabRounding = 2.0f;
		s.CellPadding = ImVec2(8, 8);
		s.WindowPadding = ImVec2(4, 4);
		s.ScrollbarRounding = 9.0f;
		s.ScrollbarSize = 15.0f;
		s.GrabMinSize = 32.0f;
		s.TabRounding = 0;
		s.WindowRounding = 4.0f;
		s.ChildRounding = 4.0f;
		s.FrameRounding = 0.0f;
		s.GrabRounding = 0;
		s.FramePadding = ImVec2(8, 4);
		s.ItemSpacing = ImVec2(8, 4);
		s.ItemInnerSpacing = ImVec2(4, 4);
		s.TabRounding = 4.0f;
		s.WindowBorderSize = 0.0f;
		s.IndentSpacing = 12.0f;
		s.ChildBorderSize = 0.0f;
		s.PopupRounding = 4.0f;
		s.FrameBorderSize = 0.0f;

		//Application& app = Application::Get();
		//GLFWwindow* window = Engine::GetCurrentWindow()->GetWindow();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}
}
