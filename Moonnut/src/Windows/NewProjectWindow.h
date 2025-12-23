#pragma once
#include <AssetManagment/Project.h>
#include <memory>

#include <imgui/imgui.h>

namespace Moon::Editor {
	enum class ProjectWindowState {
		HOME,
		CREATE
	};

	struct ProjectTemplate {
		uint32_t id;
		std::string name;
		std::string description;
		std::string iconPath;
	};

	struct RecentProject {
		std::string name;
		std::string description;
		std::string path;
		std::string key;
		bool exists;
	};

	class NewProjectWindow {
	public:
		NewProjectWindow();
		void Draw();

		std::string m_ProjectPath;

	private:
		void DrawHomeScreen();
		void DrawCreateProjectScreen();
		void DrawRecentProjects();
		void DrawProjectTemplates();
		void OpenProjectFromRecent(const RecentProject& project);
		void CreateNewProject();
		void LoadRecentProjects();
		void SaveProjectToRecent(const std::shared_ptr<Project>& project);
		void RemoveMissingProjects();

		// Helpers
		bool DrawGlowButton(const char* id, const char* label, const ImVec2& size, const ImVec4& glowColor);
		void DrawProjectCard(size_t index, const RecentProject& proj);

		// State
		ProjectWindowState m_state = ProjectWindowState::HOME;
		bool m_shouldClose = false;

		// Create Project Form
		std::string m_projectName;
		std::string m_projectDescription;
		std::string m_projectLocation;
		bool m_createInSubfolder = true;
		uint32_t m_selectedTemplate = 0;

		std::vector<ProjectTemplate> m_templates;

		std::vector<RecentProject> m_recentProjects;

		std::string m_errorMessage;
		bool m_showError = false;
	};
}