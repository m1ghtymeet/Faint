#pragma once
#include "Core/Base.h"
#include "Math/Math.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
	bool ColoredButtonV1(const char* label, const ImVec2& size, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2);
}

namespace Faint {
	namespace UI {

		static uint32_t PrimaryCol = IM_COL32(183.0f, 99.0f, 45.0f, 255);
		static uint32_t PrimaryCol2 = IM_COL32(228.0f, 63.0f, 87.0f, 255);
		static uint32_t TextCol = IM_COL32(192, 192, 192, 255);
		static ImVec2 ButtonPadding = ImVec2(16.0f, 8.0f);

		bool PrimaryButton(const std::string& name, const Vec2& size = { 0, 0 }, Color color = Color(183.0f / 255.0f, 110.0f / 255.0f, 45.0f / 255.0f, 1));
		
		bool FloatSlider(const std::string& name, float& input, float min = 0.0f, float max = 1.0f, float speed = 0.01f);
		bool CheckBox(const std::string& name, bool& value);
		void Tooltip(const std::string& message);

		inline ImFont* NormalBold;
		inline ImFont* Subtitle;
		inline ImFont* IconSolid;
	}
}