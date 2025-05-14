#include "ImUI.h"

namespace Faint {

    namespace UI {

        ImFont fbFont;

        bool PrimaryButton(const std::string& name, const Vec2& size, Color color) {
            
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 200));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 241.0f / 255.0f, 226.0f / 255.0f, 1));

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 8));

            //ImGui::PushFont(fbFont);
            const bool pressed = ImGui::Button(name.c_str(), ImVec2(size.x, size.y));

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);

            return pressed;
        }

        bool FloatSlider(const std::string& name, float& input, float min, float max, float speed)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

            const bool isUsing = ImGui::DragFloat(("##" + name).c_str(), &input, speed, min, max);

            ImGui::PopStyleVar(2);

            return isUsing;
        }

        bool CheckBox(const std::string& name, bool& value) {

            const float height = ImGui::GetTextLineHeight() + ButtonPadding.y * 2.0f;

            if (!value) {
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(98, 63, 43, 200));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, PrimaryCol);
                ImGui::PushStyleColor(ImGuiCol_Border, PrimaryCol);
            
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

                const bool buttonPressed = ImGui::Button(("##" + name).c_str(), ImVec2(height, height));

                ImGui::PopStyleVar(3);

                ImGui::PopStyleColor(4);

                if (buttonPressed)
                    value = !value;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, PrimaryCol);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PrimaryCol);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, PrimaryCol);
                ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(98, 63, 43, 200));

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

                const bool buttonPressed = ImGui::Button(("##" + name).c_str(), ImVec2(height, height));

                ImGui::PopStyleVar(3);

                ImGui::PopStyleColor(4);

                if (buttonPressed)
                    value = !value;
            }

            return value;
        }
    }
}