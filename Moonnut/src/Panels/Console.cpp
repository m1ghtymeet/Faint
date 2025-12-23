#include "Console.h"
#include <Debug/Log.h>
#include <FontAwesome5.h>

#include <imgui.h>

Console::Console(const std::string& p_name, bool p_opened, const PanelWindowSettings& p_windowSettings) :
    PanelWindow(p_name, p_opened, p_windowSettings)
{
}

void Console::_Draw_Impl() {
    // Toolbar
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
        if (ImGui::Button("Clear", ImVec2(60, 28))) {
            Moon::Log::ClearLogs();
            //m_sceneHierarchyPanel.SetStatusMessage("Clear Logs!");
        }
    
        ImGui::SameLine();
    
        if (ImGui::Button(ICON_FA_FILTER, ImVec2(30, 28))) {
            ImGui::OpenPopup("filter_popup");
        }
    
        bool isEnabled = true;
        if (ImGui::BeginPopup("filter_popup")) {
            ImGui::SeparatorText("Filters");
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
    
            if (isEnabled) {
                //glm::vec4 color = Engine::GetProject()->Settings.PrimaryColor;
                ImGui::PushStyleColor(ImGuiCol_Button, { 1, 1, 0.0f, 1.0f });
            }
    
            if (ImGui::Button((std::string(ICON_FA_BAN) + " Error").c_str())) {
                // Add filter logic
            }
    
            if (isEnabled)
                ImGui::PopStyleColor();
    
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
    }
    
    ImGui::Separator();
    
    // Log Table
    if (ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (auto& l : Moon::Log::GetLogs()) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
    
            ImGui::BeginGroup();
    
            // Time
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 255, 150, 255));
            ImGui::Text("[%s]", l.time.c_str());
            ImGui::PopStyleColor();
    
            ImGui::SameLine();
    
            // Log Name
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(98, 174, 239, 255));
            ImGui::Text(l.log.c_str());
            ImGui::PopStyleColor();
    
            ImGui::SameLine();
    
            // Log Message
            if (l.type == "trace")
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
            else if (l.type == "error") {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 0.8f));
            }
            else if (l.type == "warn") {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.1f, 0.8f));
            }
            else if (l.type == "info") {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
            }
            std::string displayMessage = l.message;
            if (l.count > 0)
                displayMessage += "(" + std::to_string(l.count) + ")";
            ImGui::TextWrapped("%s", displayMessage.c_str());
            ImGui::PopStyleColor();
    
            ImGui::EndGroup();
            ImGui::Spacing();
            ImGui::Separator();
    
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }
}
