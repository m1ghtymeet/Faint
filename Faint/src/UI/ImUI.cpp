#include "ImUI.h"

// Implementation
bool ImGui::ColoredButtonV1(const char* label, const ImVec2& size_arg, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    ImGuiButtonFlags flags = ImGuiButtonFlags_None;
    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const bool is_gradient = bg_color_1 != bg_color_2;
    if (held || hovered)
    {
        // Modify colors (ultimately this can be prebaked in the style)
        float h_increase = (held && hovered) ? 0.02f : 0.02f;
        float v_increase = (held && hovered) ? 0.20f : 0.07f;

        ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
        ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
        bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
        ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg_color_1 = GetColorU32(bg1f);
        if (is_gradient)
        {
            ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
            ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
            bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
            ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg_color_2 = GetColorU32(bg2f);
        }
        else
        {
            bg_color_2 = bg_color_1;
        }
    }
    RenderNavHighlight(bb, id);

#if 0
    // V1 : faster but prevents rounding
    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#endif

    // V2
    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color_1, g.Style.FrameRounding);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    if (is_gradient)
        ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    PushStyleColor(ImGuiCol_Text, text_color);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

namespace Faint {
    namespace UI {
        bool PrimaryButton(const std::string& label, const Vec2& size, Color color) {
            
            //ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255.0f));
            //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 200));
            //ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255));
            //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 241.0f / 255.0f, 226.0f / 255.0f, 1));

            //ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(std::size(name), 8));

            //ImGui::PushFont(fbFont);
            //const bool pressed = ImGui::Button(name.c_str(), ImVec2(size.x, size.y));

            //ImGui::PopStyleColor(4);
            //ImGui::PopStyleVar(2);

            //return pressed;
            return ImGui::ColoredButtonV1(label.c_str(), {size.x, size.y}, IM_COL32(255, 255, 255, 255), PrimaryCol, PrimaryCol2);
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

        void Tooltip(const std::string& message) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(450.0f);
                ImGui::TextUnformatted(message.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            ImGui::PopStyleVar(2);
        }
    }
}