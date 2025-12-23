#include "ProjectWindow.h"
#include "../Windows/NewProjectWindow.h"
#include <Core/GlobalLocator.h>
#include <Scene/SceneManager.h>
#include <FontAwesome5.h>
#include <UI/ImUI.h>

#include <Engine.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

Moon::Editor::NewProjectWindow* _newWindow;

ProjectWindow::ProjectWindow() {
	Moon::Settings::WindowSettings windowSettings;
	windowSettings.title = "Moon - Project Window";
	windowSettings.width = 1000;
	windowSettings.height = 580;
	windowSettings.maximized = false;
	windowSettings.resizable = false;

	m_window = std::make_unique<Moon::Window>(windowSettings);
	m_window->InitImgui();

	//Moon::GlobalLocator::Provide<Moon::SceneManager>(sceneManager);
	Moon::GlobalLocator::Provide<Moon::AssetManagment::TextureManager>(textureManager);

	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig iconsConfigBold;
	Moon::UI::NormalBold = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 30);
	Moon::UI::Subtitle = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 40);
	Moon::UI::IconSolid = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/fa-solid-900.ttf", 11, 0, icon_ranges);

	_newWindow = new Moon::Editor::NewProjectWindow();
}

ProjectWindow::~ProjectWindow() {
	m_window.reset();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::optional<std::string> ProjectWindow::Run() {
	std::optional<std::string> result;

	while (m_window->ShouldClose()) {

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		_newWindow->Draw();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		Moon::Engine::EndDraw();

		if (!_newWindow->m_ProjectPath.empty()) {
			result = _newWindow->m_ProjectPath;
			m_window->SetClose(true);
		}
		m_window->SwapBuffers();
	}
	m_window->Destroy();

	return result;
}
