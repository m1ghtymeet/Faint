#include "ProjectSettingsPanel.h"
#include <Engine.h>
#include <Renderer/Renderer.h>
#include <UI/ImUI.h>
#include <imgui.h>

int m_currentTab = 0;

using namespace Faint;
ProjectSettingsPanel::ProjectSettingsPanel() {
	categories.clear();
	categories.push_back(CreateRef<ProjectSettingsCategoryAudio>());
	categories.push_back(CreateRef<ProjectSettingsCategoryGraphics>());
	categories.push_back(CreateRef<ProjectSettingsCategoryPhysics>());
	categories.push_back(CreateRef<ProjectSettingsCategoryInputManager>());
}

void ProjectSettingsPanel::Draw(bool visible) {
	if (!visible)
		return;

	ProjectSettings settings = Engine::GetProject()->Settings;
	ImGui::SetNextWindowSizeConstraints({ 600, 300 }, { 1280, 720 });
	if (ImGui::Begin("Project settings", &visible, ImGuiWindowFlags_NoDocking)) {
		if (ImGui::BeginChild("ProjectSettingsLeft", { 200, ImGui::GetContentRegionAvail().y }, true)) {
			for (int i = 0; i < categories.size(); i++) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;
				bool is_selected = m_currentTab == i;
				if (is_selected)
					flags |= ImGuiTreeNodeFlags_Selected;

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
				ImGui::TreeNodeEx(categories.at(i)->name.c_str(), flags);

				if (ImGui::IsItemClicked()) {
					m_currentTab = i;
				}

				ImGui::TreePop();
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();
		}

		ImGui::SameLine();

		if (ImGui::BeginChild("ProjectSettingsRight", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border)) {
			ImGui::PushFont(UI::Subtitle);
			ImGui::Text(categories.at(m_currentTab)->name.c_str());
			ImGui::PopFont();
			categories.at(m_currentTab)->Draw();
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void ProjectSettingsCategoryPhysics::Draw() {
	ImGui::ColorEdit4("Primary Color", &Engine::GetProject()->Settings.PrimaryColor.r);
	
	const char* config_items[2] = {"development", "release"};
}

void ProjectSettingsCategoryGraphics::Draw() {

	const char* config_items[3] = { "OpenGL", "DirectX", "Vulkan" };
	const char* current_item = config_items[(int)Renderer::GetAPI()];
	ImGui::Text("API:");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##api", current_item)) {
		for (int n = 0; n < IM_ARRAYSIZE(config_items); n++) {
			bool is_selected = (current_item == config_items[n]);
			if (ImGui::Selectable(config_items[n], is_selected)) {
				current_item = config_items[n];
				RendererAPI api = (RendererAPI)*config_items[n];
				Renderer::SetAPI(api);
			}													
			if (is_selected)									
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void ProjectSettingsCategoryAudio::Draw() {

}

void ProjectSettingsCategoryInputManager::Draw() {

}
