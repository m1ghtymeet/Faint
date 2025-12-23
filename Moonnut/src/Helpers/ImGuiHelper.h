#pragma once
#include <string>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <FontAwesome5.h>

static bool m_lockScale = true;
static bool m_lastScaleValue = 1.0f;
static void DrawVec3Control(const std::string& label, glm::vec3* values, float resetValue = 0.0f, float columnWidth = 100.0f, bool isScale = false) {
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    // Calculate available width
    float availableWidth = ImGui::GetContentRegionAvail().x;
    bool isNarrow = availableWidth < 400.0f; // Threshold for responsive layout

    if (!isNarrow) {
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();
    }
    else {
        // Show label on separate line when narrow
        ImGui::Text("%s", label.c_str());
    }

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 4 });

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // X Component (Red)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
        values->x = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    float oldX = values->x;
    ImGui::DragFloat("##X", &values->x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y Component (Green)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
        values->y = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    float oldY = values->y;
    ImGui::DragFloat("##Y", &values->y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z Component (Blue)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
        values->z = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    float oldZ = values->z;
    ImGui::DragFloat("##Z", &values->z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    // Lock button for scale
    if (isScale) {
        ImGui::SameLine();
        const char* lockIcon = m_lockScale ? ICON_FA_LOCK : ICON_FA_UNLOCK;
        ImGui::PushStyleColor(ImGuiCol_Button, m_lockScale ?
            ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f } : ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
        if (ImGui::Button(lockIcon, buttonSize)) {
            m_lockScale = !m_lockScale;
            if (m_lockScale) {
                // When locking, store the current average
                m_lastScaleValue = (values->x + values->y + values->z) / 3.0f;
            }
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(m_lockScale ? "Unlock scale" : "Lock scale");

        // Apply locked scaling
        if (m_lockScale) {
            float delta = 0.0f;
            if (oldX != values->x) delta = values->x - oldX;
            else if (oldY != values->y) delta = values->y - oldY;
            else if (oldZ != values->z) delta = values->z - oldZ;

            if (delta != 0.0f) {
                values->x += delta;
                values->y += delta;
                values->z += delta;
            }
        }
    }

    ImGui::PopStyleVar();

    if (!isNarrow) {
        ImGui::Columns(1);
    }

    ImGui::PopID();
}