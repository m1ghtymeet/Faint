#pragma once
#include <string>
#include "Math/Math.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
	bool ColoredButtonV1(const char* label, const ImVec2& size, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2);
}

namespace Moon {
	namespace UI {

		static uint32_t PrimaryCol = IM_COL32(183.0f, 99.0f, 45.0f, 255);
		static uint32_t PrimaryCol2 = IM_COL32(228.0f, 63.0f, 87.0f, 255);
		static uint32_t TextCol = IM_COL32(192, 192, 192, 255);
		static ImVec2 ButtonPadding = ImVec2(16.0f, 8.0f);

		void ColoredText(const std::string& label, ImVec4 color = { 1, 1, 1, 1 });

		bool PrimaryButton(const std::string& name, const glm::vec2& size = { 0, 0 }, glm::vec4 color = glm::vec4(183.0f / 255.0f, 110.0f / 255.0f, 45.0f / 255.0f, 1));
		
		bool FloatSlider(const std::string& name, float& input, float min = 0.0f, float max = 1.0f, float speed = 0.01f);
		bool CheckBox(const std::string& name, bool& value);
		void Tooltip(const std::string& message);

        // 📊 Main Text Fonts
        inline ImFont* Tiny;         
        inline ImFont* Small;        
        inline ImFont* Normal;       
        inline ImFont* NormalBold;   
        inline ImFont* Section;      
        inline ImFont* Subtitle;     
        inline ImFont* Title;        
        inline ImFont* Header;       

        // 🎯 Special Fonts
        inline ImFont* Code;         
        inline ImFont* Monospace;    

        // ✨ Icon Fonts
        inline ImFont* IconTiny;     
        inline ImFont* IconSmall;    
        inline ImFont* IconSolid;    
        inline ImFont* IconMedium;   
        inline ImFont* IconLarge;    
        inline ImFont* IconHuge;     
        inline ImFont* HugeIcons;    
    }
}