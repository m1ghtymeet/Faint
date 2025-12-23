#include "NewProjectWindow.h"
#include <Debug/Assertion.h>
#include <Core/GlobalLocator.h>
#include <AssetManagment/TextureManager.h>

#include <iostream>
#include <Engine.h>
#include "Core/String.h"
#include "FileSystem/FileSystem.h"
#include "UI/ImUI.h"
#include "Util/PlatformUtil.h"
#include <FontAwesome5.h>
#include "AssetManagment/Config.h"

#include "../Panels/FileSystemUI.h"

#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>

Moon::Editor::NewProjectWindow::NewProjectWindow() {
	
	m_templates = {
		{ 0, "3D", "Standard 3D project with built-in renderer" }
	};

	LoadRecentProjects();
}

void Moon::Editor::NewProjectWindow::Draw() {
	if (m_shouldClose) return;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.11f, 1.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("##NewProjectScreen", nullptr, flags)) {
		switch (m_state) {
		case ProjectWindowState::HOME: DrawHomeScreen(); break;
		case ProjectWindowState::CREATE: DrawCreateProjectScreen(); break;
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	if (m_showError) {
		ImGui::OpenPopup("Error");
		m_showError = false;
	}

	if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s %s", "", m_errorMessage.c_str());
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Moon::Editor::NewProjectWindow::DrawHomeScreen() {
	// Gradient Background
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 winPos = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();

	// Subtle radial gradient
	ImVec2 center = ImVec2(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.3f);
	drawList->AddRectFilledMultiColor(
		winPos,
		ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
		IM_COL32(15, 15, 20, 255), // Top-left
		IM_COL32(15, 15, 20, 255), // Top-right
		IM_COL32(20, 22, 30, 255), // Bottom-right
		IM_COL32(20, 22, 30, 255)  // Bottom-left
	);

	// Floating particles/dots effect
	static float particleTime = 0.0f;
	particleTime += ImGui::GetIO().DeltaTime * 0.3f;
	for (int i = 0; i < 30; i++) {
		float x = winPos.x + (std::sin(particleTime + i * 0.5f) * 0.5f + 0.5f) * winSize.x;
		float y = winPos.y + (std::cos(particleTime * 0.7f + i * 0.3f) * 0.5f + 0.5f) * winSize.y;
		float alpha = (std::sin(particleTime * 2.0f + i) * 0.5f + 0.5f) * 0.15f;
		drawList->AddCircleFilled(ImVec2(x, y), 2.0f, IM_COL32(100, 150, 255, (int)(alpha * 255)));
	}

	ImGui::Dummy(ImVec2(0, 20));

	// Title
	ImGui::PushFont(UI::Subtitle);
	const char* titleText = "Moon Engine";
	const float titleWidth = ImGui::CalcTextSize(titleText).x;
	ImVec2 titlePos = ImVec2((ImGui::GetContentRegionAvail().x - titleWidth) * 0.5f, ImGui::GetCursorPosY());

	// Glow effect for title
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	screenPos.x = winPos.x + titlePos.x + 4.0f;
	for (int i = 0; i < 3; i++) {
		drawList->AddText(UI::Subtitle, 40.0f,
			ImVec2(screenPos.x - i, screenPos.y - i),
			IM_COL32(80, 150, 255, 30), titleText);
		drawList->AddText(UI::Subtitle, 40.0f,
			ImVec2(screenPos.x + i, screenPos.y + i),
			IM_COL32(80, 150, 255, 30), titleText);
	}

	ImGui::SetCursorPosX(titlePos.x);
	ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", titleText);
	ImGui::PopFont();

	ImGui::Dummy(ImVec2(0, 40));

	// Action Buttons
	const float buttonWidth = 200.0f;
	const float buttonHeight = 40.0f;
	const float spacing = 20.0f;
	const float totalWidth = buttonWidth * 2 + spacing;
	const float startX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

	ImGui::SetCursorPosX(startX);

	if (DrawGlowButton("##CreateProject", "+ New Project", ImVec2(buttonWidth, buttonHeight), ImVec4(0.2f, 0.6f, 1.0f, 1.0f))) {
		m_state = ProjectWindowState::CREATE;
		m_projectName.clear();
		m_projectDescription.clear();
		m_projectLocation.clear();
		m_selectedTemplate = 0;
	}
	ImGui::SameLine();
	if (DrawGlowButton("##OpenProject", "Open Project", ImVec2(buttonWidth, buttonHeight), ImVec4(0.5f, 0.3f, 0.9f, 1.0f))) {
		std::string path = FileDialogs::OpenFile("Moon Project (*.project)\0*.project\0All Files\0*.*\0");
		if (!path.empty() && FileSystem::FileExists(path, true)) {
			auto project = Project::Load(path);
			if (project) {
				Engine::LoadProject(project);
				SaveProjectToRecent(project);
				m_ProjectPath = project->FullPath;
				m_shouldClose = true;
			}
		}
	}

	ImGui::Dummy(ImVec2(0, 40));

	DrawRecentProjects();
}

void Moon::Editor::NewProjectWindow::DrawRecentProjects() {
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);

	if (ImGui::BeginChild("RecentProjectsView",
		ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.6f),
		false, ImGuiChildFlags_AlwaysUseWindowPadding)) {

		// Title
		ImGui::PushFont(UI::Subtitle);
		ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "Recent Projects");
		ImGui::PopFont();

		ImGui::Dummy(ImVec2(0, 12));

		// Glass-morphism Container
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 containerPos = ImGui::GetCursorScreenPos();
		ImVec2 containerSize = ImVec2(ImGui::GetContentRegionAvail().x - 32.0f, ImGui::GetContentRegionAvail().y);

		// Glass background with blur effect
		drawList->AddRectFilled(
			containerPos,
			ImVec2(containerPos.x + containerSize.x, containerPos.y + containerSize.y),
			IM_COL32(25, 28, 35, 180), 16.0f);

		// Glass border
		drawList->AddRect(
			containerPos,
			ImVec2(containerPos.x + containerSize.x, containerPos.y + containerSize.y),
			IM_COL32(80, 120, 180, 60), 16.0f, 0, 1.5f);

		if (ImGui::BeginChild("ProjectContainer", containerSize, false, ImGuiChildFlags_AlwaysUseWindowPadding)) {
			if (m_recentProjects.empty()) {
				ImGui::Dummy(ImVec2(0, containerSize.y));
				const char* emptyText = "No recent projects";
				float textWidth = ImGui::CalcTextSize(emptyText).x;
				ImGui::SetCursorPosX((containerSize.x - textWidth) * 0.5f);
				ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 0.5f), "%s", emptyText);
			}
			else {
				ImGui::Dummy(ImVec2(0, 8));
				for (size_t i = 0; i < m_recentProjects.size(); i++) {
					ImGui::PushID(static_cast<int>(i));

					const auto& proj = m_recentProjects[i];
					if (proj.exists) {
						DrawProjectCard(i, proj);
					}
					else {
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "(Missing)");
					}

					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void Moon::Editor::NewProjectWindow::DrawProjectTemplates() {
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 0.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

	if (ImGui::BeginChild("Templates", ImVec2(0, 180), true)) {
		for (const auto& tmpl : m_templates) {
			ImGui::PushID(tmpl.id);

			const bool isSelected = (m_selectedTemplate == tmpl.id);
			ImGui::PushStyleColor(ImGuiCol_Header,
				isSelected ? ImVec4(0.25f, 0.5f, 0.8f, 0.4f) : ImVec4(0.2f, 0.2f, 0.2f, 0.3f));

			if (ImGui::Selectable(("##Template" + std::to_string(tmpl.id)).c_str(),
				isSelected, 0, ImVec2(0, 50))) {
				m_selectedTemplate = tmpl.id;
			}

			ImGui::PopStyleColor();

			// Draw template info
			ImVec2 pos = ImGui::GetItemRectMin();
			ImGui::SetCursorPos(ImVec2(pos.x + 12, pos.y + 8));

			ImGui::PushFont(UI::NormalBold);
			ImGui::TextColored(isSelected ? ImVec4(0.5f, 0.8f, 1.0f, 1.0f) : ImVec4(1, 1, 1, 1),
				"%s", tmpl.name.c_str());
			ImGui::PopFont();

			ImGui::SetCursorPosX(pos.x + 12);
			ImGui::TextColored(ImVec4(1, 1, 1, 0.6f), "%s", tmpl.description.c_str());

			ImGui::PopID();
		}
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void Moon::Editor::NewProjectWindow::DrawCreateProjectScreen() {
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);

	ImGui::BeginChild("##CreateForm", ImVec2(0, 0), false, ImGuiChildFlags_AlwaysUseWindowPadding);
	
	// Title
	ImGui::PushFont(UI::Subtitle);
	ImGui::Text("Create a New Project");
	ImGui::PopFont();

	ImGui::Dummy(ImVec2(0, 20));

	// Error message
	if (m_showError && !m_errorMessage.empty()) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		ImGui::TextWrapped("%s", m_errorMessage.c_str());
		ImGui::PopStyleColor();
		ImGui::Dummy(ImVec2(0, 10));
	}

	// Project name
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Project Name");
	ImGui::SameLine();
	if (m_projectName.empty() && m_showError) {
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "*");
		UI::Tooltip("Project name is required");
	}
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##ProjectName", &m_projectName);
	ImGui::Dummy(ImVec2(0, 16));

	// Description
	ImGui::Text("Description");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 1, 0.4f), "(Optional)");
	const float inputHeight = ImGui::GetTextLineHeight() * 4 + ImGui::GetStyle().FramePadding.y * 2;
	ImGui::SetNextItemWidth(-1);
	ImGui::InputTextMultiline("##Description", &m_projectDescription);
	ImGui::Dummy(ImVec2(0, 16));

	// Location
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Location");
	ImGui::SameLine();

	const bool isLocationEmpty = m_projectLocation.empty();
	const bool isPathValid = !isLocationEmpty && FileSystem::DirectoryExists(m_projectLocation, true);

	if (m_showError && (isLocationEmpty || !isPathValid)) {
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "*");
		if (isLocationEmpty) {
			UI::Tooltip("Location is required");
		}
		else {
			UI::Tooltip("Selected path does not exist");
		}
	}

	ImGui::TextColored(ImVec4(1, 1, 1, 0.4f), "Folder where the project will be created");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
	ImGui::InputText("##Location", &m_projectLocation);
	ImGui::SameLine();
	if (ImGui::Button("...")) {
		std::string path = FileDialogs::SaveFile("Project File\0*.project");
		if (!path.empty()) {
			m_projectLocation = String::ReplaceSlash(path);
		}
	}

	// Final Path Preview
	if (!m_projectLocation.empty()) {
		std::string finalPath = m_projectLocation;
		if (m_createInSubfolder && !m_projectName.empty()) {
			finalPath += "/" + m_projectName;
		}
		finalPath += "/" + (m_projectName.empty() ? "project.project" : m_projectName + ".project");

		ImGui::Dummy(ImVec2(0, 8));
		ImGui::TextColored(ImVec4(1, 1, 1, 0.4f), "Project will be created at:");
		ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%s", finalPath.c_str());
	}

	ImGui::Dummy(ImVec2(0, 20));

	// Options
	ImGui::Text("Options");
	ImGui::Dummy(ImVec2(0, 8));
	ImGui::Checkbox("Create in subfolder", &m_createInSubfolder);
	UI::Tooltip("Creates a dedicated folder for the project inside the selected location");

	ImGui::Dummy(ImVec2(0, 20));

	// Templates (Optional Section)
	//ImGui::Text("Template");
	//ImGui::Dummy(ImVec2(0, 8));
	//DrawProjectTemplates();
	//
	//ImGui::Dummy(ImVec2(0, 32));

	// Action Buttons
	const float buttonWidth = 120.0f;
	const float buttonHeight = 38.0f;
	const float spacing = 12.0f;

	if (ImGui::Button("Cancel", ImVec2(buttonWidth, buttonHeight))) {
		m_state = ProjectWindowState::HOME;
		m_errorMessage.clear();
		m_showError = false;
	}
	ImGui::SameLine();

	const float availWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonWidth);

	if (ImGui::Button("+ Create", ImVec2(buttonWidth, buttonHeight))) {
		CreateNewProject();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void Moon::Editor::NewProjectWindow::OpenProjectFromRecent(const RecentProject& project) {
	try {
		auto loadedProject = Project::Load(project.path);
		if (loadedProject) {
			Engine::LoadProject(loadedProject);
			m_ProjectPath = project.path;
			m_shouldClose = true;
		}
	}
	catch (const std::exception& e) {
		m_errorMessage = std::string("Failed to open project: ") + e.what();
		m_showError = true;
		HZ_CORE_ERROR(m_errorMessage);
	}
}

void Moon::Editor::NewProjectWindow::CreateNewProject() {
	m_showError = true;

	// Validation
	if (m_projectName.empty()) {
		m_errorMessage = "Project name cannot be empty";
		return;
	}

	if (m_projectLocation.empty()) {
		m_errorMessage = "Project location must be specified";
		return;
	}

	if (!FileSystem::DirectoryExists(m_projectLocation, true)) {
		m_errorMessage = "Selected location does not exist";
		return;
	}

	// Build final path
	std::string finalPath = m_projectLocation;
	if (m_createInSubfolder) {
		finalPath += "/" + m_projectName;
	}
	finalPath += "/" + m_projectName + ".project";

	const std::string projectDir = std::filesystem::path(finalPath).parent_path().string();

	try {
		// Create directory
		if (!FileSystem::DirectoryExists(projectDir, true)) {
			if (!std::filesystem::create_directories(projectDir)) {
				m_errorMessage = "Failed to create project directory";
				return;
			}
		}

		// Create project
		auto project = std::make_shared<Project>(m_projectName, m_projectDescription, finalPath);
		FileSystem::SetRootDirectory(projectDir + "/");

		// Create essential directories
		const std::string assetsDir = FileSystem::Root + "Assets/";
		const std::string scriptsDir = FileSystem::Root + "Scripts/";

		std::filesystem::create_directories(assetsDir);
		std::filesystem::create_directories(scriptsDir);

		project->AssetDirectory = assetsDir;
		project->Save();

		// Save to recent
		SaveProjectToRecent(project);

		// Load project
		Engine::LoadProject(project);
		m_ProjectPath = finalPath;
		m_shouldClose = true;

		FileBrowser::m_currentDirectory = FileSystem::RootDirectory;

	}
	catch (const std::exception& e) {
		m_errorMessage = std::string("Failed to create project: ") + e.what();
		HZ_CORE_ERROR(m_errorMessage);
	}
}

void Moon::Editor::NewProjectWindow::LoadRecentProjects() {
	m_recentProjects.clear();

	try {
		mINI::INIFile file = Config::Begin("config/projects.ini", true);
		auto& section = Config::currentINI["projects"];

		for (size_t i = 0; i < section.size(); i++) {
			std::string projectKey = section[std::to_string(i)];

			RecentProject proj;
			proj.name = Config::currentINI[projectKey]["name"];
			proj.description = Config::currentINI[projectKey]["description"];
			proj.path = Config::currentINI[projectKey]["path"];
			proj.key = projectKey;
			proj.exists = FileSystem::DirectoryExists(
				FileSystem::GetParentPath(proj.path), true);

			m_recentProjects.push_back(proj);
		}
	}
	catch (const std::exception& e) {
		HZ_CORE_WARN(std::string("Failed to load recent projects: ") + e.what());
	}
}

void Moon::Editor::NewProjectWindow::SaveProjectToRecent(const std::shared_ptr<Project>& project) {
	//try {
	//	std::string configPath = "config/projects.ini";
	//	mINI::INIFile file = Config::Begin(configPath, true);
	//
	//	auto& ini = Config::currentINI;
	//	auto& projectsSection = ini["projects"];
	//
	//	std::string safeName = project->Name;
	//	std::transform(safeName.begin(), safeName.end(), safeName.begin(), ::tolower);
	//	for (char& c : safeName) {
	//		if (!std::isalnum(c) && c != '_') c = '_';
	//	}
	//	std::string projectKey = "projects_" + safeName;
	//
	//	if (ini.has(projectKey)) {
	//		ini.remove(projectKey);
	//	}
	//
	//	for (auto it = projectsSection.begin(); it != projectsSection.end(); ) {
	//		//if (it->second == projectKey) {
	//		//	it = projectsSection.remove(it);
	//		//}
	//		//else {
	//		//	++it;
	//		//}
	//	}
	//
	//	auto& projectSection = ini[projectKey];
	//	projectSection.set("name", project->Name);
	//	projectSection.set("description", project->Description.empty() ? "" : project->Description);
	//	projectSection.set("path", project->FullPath);
	//
	//	size_t index = 0;
	//	while (projectsSection.has(std::to_string(index))) {
	//		++index;
	//	}
	//
	//	projectsSection.set(std::to_string(index), projectKey);
	//
	//	file.write(ini, true); // true = pretty print
	//}
	//catch (const std::exception& e) {
	//	HZ_CORE_WARN("Failed to save project to recent list: {}", e.what());
	//}
}

void Moon::Editor::NewProjectWindow::RemoveMissingProjects()
{
}

bool Moon::Editor::NewProjectWindow::DrawGlowButton(const char* id, const char* label, const ImVec2& size, const ImVec4& glowColor) {
	static std::map<std::string, float> buttonAnimations;
	std::string buttonId(id);
	
	if (buttonAnimations.find(buttonId) == buttonAnimations.end()) {
		buttonAnimations[buttonId] = 0.0f;
	}
	
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 endPos = ImVec2(pos.x + size.x, pos.y + size.y);
	
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	
	bool clicked = ImGui::InvisibleButton(id, size);
	bool hovered = ImGui::IsItemHovered();
	bool active = ImGui::IsItemActive();
	
	// Animation
	float targetAnim = (hovered || active) ? 1.0f : 0.0f;
	buttonAnimations[buttonId] += (targetAnim - buttonAnimations[buttonId]) * 0.2f;
	float anim = buttonAnimations[buttonId];
	
	// Outer glow
	if (anim > 0.01f) {
		for (int i = 5; i > 0; i--) {
			int alpha = (int)(anim * 40.0f / i);
			drawList->AddRectFilled(
				ImVec2(pos.x - i * 2, pos.y - i * 2),
				ImVec2(endPos.x + i * 2, endPos.y + i * 2),
				IM_COL32(
					(int)(glowColor.x * 255),
					(int)(glowColor.y * 255),
					(int)(glowColor.z * 255),
					alpha),
				12.0f + i);
		}
	}
	
	// Button background gradient
	ImU32 bgColor1 = IM_COL32(
		(int)(35 + anim * 20),
		(int)(38 + anim * 22),
		(int)(45 + anim * 25),
		255);
	ImU32 bgColor2 = IM_COL32(
		(int)(25 + anim * 15),
		(int)(28 + anim * 17),
		(int)(35 + anim * 20),
		255);
	
	//drawList->AddRectFilledMultiColor(pos, endPos,
	//	bgColor1, bgColor1, bgColor2, bgColor2);
	
	// Border with glow color
	int borderAlpha = (int)(100 + anim * 155);
	drawList->AddRect(pos, endPos,
		IM_COL32(
			(int)(glowColor.x * 255),
			(int)(glowColor.y * 255),
			(int)(glowColor.z * 255),
			borderAlpha),
		12.0f, 0, 2.0f);
	
	// Inner highlight
	if (anim > 0.01f) {
		drawList->AddRectFilled(
			ImVec2(pos.x + 2, pos.y + 2),
			ImVec2(endPos.x - 2, pos.y + size.y * 0.3f),
			IM_COL32(255, 255, 255, (int)(anim * 20)), 10.0f);
	}
	
	// Icon and text
	const float iconSize = 32.0f;
	const float padding = 16.0f;
	
	ImVec2 iconPos = ImVec2(
		pos.x + padding,
		pos.y + (size.y - iconSize) * 0.5f);
	
	// Load and draw icon
	//auto* iconTexture = FTSERVICE(Moon::AssetManagment::TextureManager)
	//	.CreateResource(iconPath);
	//
	//if (iconTexture) {
	//	// Icon glow
	//	if (anim > 0.01f) {
	//		for (int i = 2; i > 0; i--) {
	//			drawList->AddCircleFilled(
	//				ImVec2(iconPos.x + iconSize * 0.5f, iconPos.y + iconSize * 0.5f),
	//				iconSize * 0.5f + i * 3,
	//				IM_COL32(
	//					(int)(glowColor.x * 255),
	//					(int)(glowColor.y * 255),
	//					(int)(glowColor.z * 255),
	//					(int)(anim * 40 / i)));
	//		}
	//	}
	//
	//	drawList->AddImage(
	//		(void*)iconTexture->GetTexture().GetID(),
	//		iconPos,
	//		ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
	//		ImVec2(0, 1), ImVec2(1, 0),
	//		IM_COL32(255, 255, 255, 255));
	//}
	
	// Text
	ImGui::PushFont(UI::NormalBold);
	ImVec2 textSize = ImGui::CalcTextSize(label);
	ImVec2 textPos = ImVec2(
		pos.x + padding * 2,
		pos.y + (size.y - textSize.y) * 0.5f);
	
	// Text glow
	if (anim > 0.01f) {
		for (int i = 0; i < 2; i++) {
			drawList->AddText(UI::NormalBold,
				ImGui::GetFontSize(),
				ImVec2(textPos.x - i, textPos.y - i),
				IM_COL32(
					(int)(glowColor.x * 255),
					(int)(glowColor.y * 255),
					(int)(glowColor.z * 255),
					(int)(anim * 60)),
				label);
			drawList->AddText(UI::NormalBold,
				ImGui::GetFontSize(),
				ImVec2(textPos.x + i, textPos.y + i),
				IM_COL32(
					(int)(glowColor.x * 255),
					(int)(glowColor.y * 255),
					(int)(glowColor.z * 255),
					(int)(anim * 60)),
				label);
		}
	}
	
	// Main text
	drawList->AddText(UI::NormalBold,
		ImGui::GetFontSize(),
		textPos,
		IM_COL32(
			(int)((0.85f + anim * 0.15f) * 255),
			(int)((0.90f + anim * 0.10f) * 255),
			(int)((0.95f + anim * 0.05f) * 255),
			255),
		label);
	
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar();
	
	return clicked;
}

void Moon::Editor::NewProjectWindow::DrawProjectCard(size_t index, const RecentProject& proj) {
	const float cursorYStart = ImGui::GetCursorPosY();
	const float cardHeight = 64.0f;
	const float padding = 16.0f;
	const float iconSize = 48.0f;

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 cardStart = ImGui::GetCursorScreenPos();
	ImVec2 cardEnd = ImVec2(cardStart.x + ImGui::GetContentRegionAvail().x, cardStart.y + cardHeight);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 12));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));

	bool clicked = ImGui::Selectable(("##Project" + std::to_string(index)).c_str(),
		false, ImGuiSelectableFlags_AllowDoubleClick,
		ImVec2(ImGui::GetContentRegionAvail().x, cardHeight));

	bool isHovered = ImGui::IsItemHovered();
	static size_t hoveredIndex = (size_t)-1;
	static float hoverAnim[32] = { 0 };

	if (isHovered) hoveredIndex = index;

	// Smooth animation
	float targetAnim = (isHovered || hoveredIndex == index) ? 1.0f : 0.0f;
	hoverAnim[index] += (targetAnim - hoverAnim[index]) * 0.15f;

	if (clicked)
		OpenProjectFromRecent(proj);

	// Card background with gradient
	ImU32 bgColor1 = IM_COL32(
		30 + (int)(hoverAnim[index] * 25),
		32 + (int)(hoverAnim[index] * 28),
		40 + (int)(hoverAnim[index] * 35),
		200);
	ImU32 bgColor2 = IM_COL32(
		25 + (int)(hoverAnim[index] * 20),
		27 + (int)(hoverAnim[index] * 23),
		35 + (int)(hoverAnim[index] * 30),
		200);

	drawList->AddRectFilledMultiColor(cardStart, cardEnd,
		bgColor1, bgColor1, bgColor2, bgColor2);

	// Glow border on hover
	if (hoverAnim[index] > 0.01f) {
		for (int i = 0; i < 3; i++) {
			float alpha = hoverAnim[index] * (40 - i * 10);
			drawList->AddRect(
				ImVec2(cardStart.x - i, cardStart.y - i),
				ImVec2(cardEnd.x + i, cardEnd.y + i),
				IM_COL32(80, 150, 255, (int)alpha), 12.0f, 0, 2.0f);
		}
	}

	// Inner border
	drawList->AddRect(cardStart, cardEnd,
		IM_COL32(60 + (int)(hoverAnim[index] * 80),
			80 + (int)(hoverAnim[index] * 100),
			120 + (int)(hoverAnim[index] * 135),
			100 + (int)(hoverAnim[index] * 100)),
		12.0f, 0, 1.5f);

	// Icon with glow
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
	ImGui::SetCursorPosY(cursorYStart + (cardHeight - iconSize) / 2);

	ImVec2 iconPos = ImGui::GetCursorScreenPos();

	// Icon glow
	//if (hoverAnim[index] > 0.01f) {
	//	for (int i = 3; i > 0; i--) {
	//		drawList->AddCircleFilled(
	//			ImVec2(iconPos.x + iconSize * 0.5f, iconPos.y + iconSize * 0.5f),
	//			iconSize * 0.5f + i * 4,
	//			IM_COL32(80, 150, 255, (int)(hoverAnim[index] * 30 / i)));
	//	}
	//}

	Moon::Assets::Texture* tex = FTSERVICE(Moon::AssetManagment::TextureManager)
		.LoadResource("data/editor/icons/FileIcon.png");
	if (tex)
		ImGui::Image((ImTextureID)(uintptr_t)tex->GetTexture().GetID(), ImVec2(iconSize, iconSize),
			ImVec2(0, 1), ImVec2(1, 0));
	else {
		// Fallback: draw a custom icon
		drawList->AddRectFilled(iconPos,
			ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
			IM_COL32(60, 100, 160, 255), 8.0f);
		drawList->AddRect(iconPos,
			ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
			IM_COL32(100, 150, 220, 255), 8.0f, 0, 2.0f);
	}

	// Text
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
	ImGui::BeginGroup();
	{
		ImGui::SetCursorPosY(cursorYStart + padding - 4);
		ImGui::PushFont(UI::NormalBold);

		ImVec4 titleColor = ImVec4(
			0.85f + hoverAnim[index] * 0.15f,
			0.90f + hoverAnim[index] * 0.10f,
			0.95f + hoverAnim[index] * 0.05f,
			1.0f);
		ImGui::TextColored(titleColor, "%s", proj.name.c_str());
		ImGui::PopFont();

		ImGui::SetCursorPosY(cursorYStart + padding + 18.0f);
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x - padding * 2);
		ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.85f, 0.7f),
			"%s", proj.description.c_str());
		ImGui::PopTextWrapPos();
	}
	ImGui::EndGroup();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::SetCursorPosY(cursorYStart + cardHeight + 12.0f);
}
