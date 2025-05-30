#pragma once
#include "Scene/Entity.h"

#include "UI/ImUI.h"
#include "imgui/imgui.h"

#define MenuItemComponent(label, component) \
	if (entity.HasComponent<component>())   \
		ImGui::Text(label)					\
	else if (ImGui::MenuItem(label))		\
		entity.AddComponent<component>();

//entity.RemoveComponent<component>(); 
//delete boldFont;

#define BeginComponentTable(name, component)											\
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));					\
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f));				\
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);								\
	bool removed = false;																\
	bool headerOpened = ImGui::CollapsingHeader(#name, ImGuiTreeNodeFlags_DefaultOpen); \
	ImGui::PopStyleVar();																\
	if (strcmp(#name, "TRANSFORM") != 0 && ImGui::BeginPopupContextItem()) {			\
		if (ImGui::Selectable("Remove")) { removed = true; }							\
		ImGui::EndPopup();																\
	}																					\
																						\
	if (removed) {																		\
		ImGui::PopStyleVar();															\
	}																					\
	else if (headerOpened) {															\
																						\
		ImGui::PopStyleVar();															\
		ImGui::Indent();																\
		if (ImGui::BeginTable(#name, 3, ImGuiTableFlags_SizingStretchProp)) {			\
			ImGui::TableSetupColumn("name", 0, 0.25f);									\
			ImGui::TableSetupColumn("set", 0, 0.65f);									\
			ImGui::TableSetupColumn("reset", 0, 0.1f);									\
			ImGui::TableNextColumn();													\

#define EndComponentTable()				\
			ImGui::EndTable();          \
        }                               \
        ImGui::Unindent();              \
    }                                   \
    else                                \
    {                                   \
        ImGui::PopStyleVar();           \
                        \
    }                                   \
    ImGui::PopStyleVar();				\

#define ComponentTableReset(setting, value)										\
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));					\
	std::string resetLabel = std::string(ICON_FA_UNDO) + "##Reset" + #setting;	\
	if (ImGui::Button(resetLabel.c_str())) setting = value;						\
		ImGui::PopStyleColor();

#define ComponentDropDown(arrayData, enumData, value)			\
	const char* current_item = arrayData[(int)value];			\
	if (ImGui::BeginCombo("Type", current_item)) {				\
		for (int n = 0; n < IM_ARRAYSIZE(arrayData); n++) {		\
			bool is_selected = (current_item == arrayData[n]);	\
			if (ImGui::Selectable(arrayData[n], is_selected)) { \
				current_item = arrayData[n];					\
				value = (enumData)n;							\
			}													\
			if (is_selected)									\
				ImGui::SetItemDefaultFocus();					\
		}														\
		ImGui::EndCombo();										\
	}

class ComponentPanel {
public:
	virtual void Draw(Faint::Entity entity);
};

inline void ComponentPanel::Draw(Faint::Entity entity) {}