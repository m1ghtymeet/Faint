#include "Window.h"

#include "Math/Math.h"
#include "Scene/Scene.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

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

	Ref<Window> Window::Get() {
		static Ref<Window> instance;
		if (instance == nullptr) {
			instance = CreateRef<Window>();
		}

		return instance;
	}

	GLFWwindow* Window::GetWindow() {
		return window;
	}

	bool Window::ShouldClose() {
		return !glfwWindowShouldClose(window);
	}

	static void GLFWErrorCallback(int error, const char* description) {
		HZ_CORE_ERROR("GLFW Error " + std::to_string(error) + ": " + description + "");
	}

	void Window::Init() {
		if (!glfwInit()) {
			HZ_CORE_ERROR("GLFW Initialization Failed!");
		}

		glfwSetErrorCallback(GLFWErrorCallback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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

		//glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		InitImgui();

		glClearColor(0, 0, 0, 1);
		glEnable(GL_DEPTH_TEST);
	}

	void Window::Update(Time time)
	{
		scene->Update(time);
	}

	void Window::FixedUpdate(Time time)
	{
		scene->FixedUpdate(time);
	}

	void Window::Draw() {
		HZ_PROFILE_FUNCTION();
		if (!scene) return;
		Ref<Camera> cam = scene->GetCurrentCamera();

		if (Engine::IsPlayMode()) {
			scene->Draw();
		}
		else {
			//float resolutionScale = glm::clamp(Engine::GetProject()->Settings.ResolutionScale, 0.5f, 2.0f);
			Engine::GetCurrentScene()->m_EditorCamera->UpdateProjection(viewportWidth, viewportHeight);
			scene->Draw(scene->m_EditorCamera->GetProjectionMatrix(), scene->m_EditorCamera->GetViewMatrix());
		}
		Renderer::EndScene();
	}

	void Window::EndDraw() {
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

	void Window::SetScene(Ref<Scene> scene) {
		this->scene = scene;
	}

	Ref<Scene> Window::GetScene() {
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
		stbi_set_flip_vertically_on_load(false);
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

		const ImVec4 darkCarbon(0.10f, 0.10f, 0.12f, 1.00f);      // خاکستری تیره

		style.WindowPadding = ImVec2(15, 15);
		style.WindowRounding = 5.0f;
		style.FramePadding = ImVec2(5, 5);
		style.FrameRounding = 4.0f;
		style.ItemSpacing = ImVec2(12, 8);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 25.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 3.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;

		colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		//colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		//style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		//style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		//style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		//style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		//style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		//style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		//style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}
}
