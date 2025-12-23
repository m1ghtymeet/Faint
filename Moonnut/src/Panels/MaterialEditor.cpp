#include "MaterialEditor.h"
#include <Core/GlobalLocator.h>
#include <Core/OS.h>
#include <Debug/Log.h>
#include <AssetManagment/MeshManager.h>
#include <AssetManagment/TextureManager.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <Scene/Scene.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "../Core/PanelsManager.h"
#include "FileSystemUI.h"

namespace Moon::Editor {
	Moon::Assets::Texture* MaterialEditor::s_emptyTexture = nullptr;

	std::string PrettyName(const std::string& name) {
		std::string result = name;

		size_t underscorePos = result.find('_');
		if (underscorePos != std::string::npos)
			result = result.substr(underscorePos + 1);

		std::string formatted;
		formatted.reserve(result.size());
		for (size_t i = 0; i < result.size(); ++i) {
			if (i > 0 && std::isupper(static_cast<unsigned char>(result[i])) && std::islower(static_cast<unsigned char>(result[i - 1]))) {
				formatted += ' ';
			}
			formatted += result[i];
		}

		return formatted;
	}
}

Moon::Editor::MaterialEditor::MaterialEditor(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_title, p_opened, p_windowSettings) {

	if (!m_material)
		SetMaterial(*Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true));

	if (!s_emptyTexture)
		s_emptyTexture = Moon::Loaders::TextureLoader::Create("data/editor/icons/Empty_Texture.png");
}

void Moon::Editor::MaterialEditor::SetMaterial(Moon::Rendering::Material& p_material, bool p_liveEdit) {
	m_material = &p_material;
	m_shader = m_material->GetShader().get();
	m_liveEdit = p_liveEdit;
	m_propertiesDirty = true;
}

void Moon::Editor::MaterialEditor::SetScene(Moon::Scene& p_scene) {
	if (IsOpened())
		m_scene = &p_scene;
}

void Moon::Editor::MaterialEditor::Update(float deltaTime) {
	//m_previewRotation += deltaTime * 0.5f;
	//if (m_previewRotation > 360.0f)
	//	m_previewRotation -= 360.0f;
}

void Moon::Editor::MaterialEditor::DrawTextureSlot(const TextureSlot& slot, float size) {
	// ProfileFunction

	ImGui::BeginGroup();

	// Get current texture
	Moon::Assets::Texture* texture = nullptr;
	if (auto prop = m_material->GetProperty(slot.name)) {
		if (std::holds_alternative<Moon::Assets::Texture*>(prop->value))
			texture = std::get<Moon::Assets::Texture*>(prop->value);
	}

	if (!texture)
		texture = s_emptyTexture;

	ImGui::PushID(slot.name.c_str());

	ImGui::PushStyleColor(ImGuiCol_Button, { slot.tintColor.x, slot.tintColor.y, slot.tintColor.z, slot.tintColor.w });
	if (ImGui::ImageButton(
		(ImTextureID)texture->GetTexture().GetID(),
		ImVec2(size, size),
		ImVec2(0, 1), ImVec2(1, 0), 1)) {
		// TODO: Handle texture selection
	}
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Load Texture")) {
			// TODO: Open file dialog
		}
		if (texture != s_emptyTexture && ImGui::MenuItem("Clear")) {
			m_material->RemoveProperty(slot.name);
			m_material->SetProperty("u_Has" + slot.displayName, false);
		}
		if (texture != s_emptyTexture && ImGui::MenuItem("Show in Explorer")) {
			// TODO: Open explorer
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TEXTURE")) {
			std::string droppedPath = static_cast<const char*>(payload->Data);
			Moon::Assets::Texture* newTexture = FTSERVICE(Moon::AssetManagment::TextureManager).CreateResource(droppedPath);
			if (newTexture) {
				m_material->SetProperty(slot.name, newTexture);
				m_material->SetProperty("u_Has" + slot.displayName, true);
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();

	ImGui::TextWrapped("%s", slot.displayName.c_str());

	// Texture info tooltip
	if (ImGui::IsItemHovered() && texture != s_emptyTexture) {
		ImGui::BeginTooltip();
		ImGui::Text("Path: %s", texture->path.c_str());
		ImGui::Text("Size: %dx%d", texture->GetTexture().GetData().width, texture->GetTexture().GetData().height);
		ImGui::Text("Use Mipmap: %d", texture->GetTexture().GetData().useMipMaps);
		ImGui::EndTooltip();
	}

	ImGui::EndGroup();
}

void Moon::Editor::MaterialEditor::DrawTextureGrid() {
	// ProfileFunction

	const float textureSize = 100.0f;
	const int columns = 3;
	int currentColumn = 0;

	for (const auto& slot : m_textureSlots) {
		if (currentColumn > 0) {
			ImGui::SameLine();
		}

		DrawTextureSlot(slot, textureSize);

		currentColumn = (currentColumn + 1) % columns;

		// Draw toggle if exists
		//if (slot.hasToggle) {
		//	std::string toggleName = "u_Has" + slot.displayName;
		//	if (auto prop = m_material->GetProperty(toggleName)) {
		//		if (std::holds_alternative<bool>(prop->value)) {
		//			bool hasTexture = std::get<bool>(prop->value);
		//			if (ImGui::Checkbox(("Enabled##" + slot.displayName).c_str(), &hasTexture)) {
		//				m_material->SetProperty(toggleName, hasTexture);
		//			}
		//		}
		//	}
		//}

		// Add some spacing between rows
		if (currentColumn == 0) {
			ImGui::Spacing();
			ImGui::Spacing();
		}
	}
}

void Moon::Editor::MaterialEditor::_Draw_Impl() {
	// ProfileFunction

	if (!m_material || !m_material->IsValid()) {
		ImGui::TextColored({ 1, 0, 0, 1 }, "No valid material selected");
		return;
	}

	// Header
	{
		// Material info
		ImGui::Text("Material: %s", m_material->path.c_str());
		ImGui::Text("Shader: %s", m_shader ? m_shader->GetPath().c_str() : "None");

		ImGui::Spacing();

		// Action buttons
		ImGui::BeginGroup();
		if (ImGui::Button("Save", ImVec2(80, 0))) {
			if (m_material->path.empty()) {
				// TODO: Need to implement a save dialog
			}
			Moon::Loaders::MaterialLoader::Save(*m_material, m_material->path);
			FTSERVICE(PanelsManager).GetPanelAs<FileBrowser>("File Browser").MakeDirty(m_material->path);
		
			if (!m_liveEdit) {
				for (auto modelRenderers : m_scene->GetFastAccessComponents().modelRenderers) {
					Moon::MaterialRendererComp* materialComp = modelRenderers->owner.GetComponent<Moon::MaterialRendererComp>();
		
					for (auto mesh : modelRenderers->GetModel()->GetMeshIndices()) {
						uint32_t matIndex = AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex;
		
						auto* currentMat = materialComp->GetMaterialByIndex(matIndex);
						if (currentMat && currentMat->path == m_material->path) {
							materialComp->RemoveMaterialByIndex(matIndex);
							materialComp->AddMaterialByIndex(matIndex, *m_material);
						}
					}
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload", ImVec2(80, 0))) {
		}
		ImGui::SameLine();
		if (ImGui::Button("Hotload", ImVec2(80, 0))) {
			if (m_shader) {
				m_shader->Hotload();
				HZ_CORE_INFO("Shader hotloaded: " + m_shader->GetPath());
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Edit Shader", ImVec2(100, 0))) {
			Moon::OS::OpenIn(m_shader->GetPath());
		}
		ImGui::EndGroup();

		ImGui::Spacing();
		if (ImGui::Button("Drop Shader Here", ImVec2(150, 0))) {
			// TODO: Change Shader Module
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SHADER")) {
				std::string shaderPath = static_cast<const char*>(payload->Data);
				std::shared_ptr<Moon::Shader> newShader = std::make_shared<Moon::Shader>(std::vector<std::string>{ shaderPath });
				if (newShader) {
					m_shader = newShader.get();
					m_material->SetShader(newShader);
					HZ_CORE_INFO("Shader changed to: " + shaderPath);
				}
				else {
					HZ_CORE_INFO("Failed to load shader: " + shaderPath);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Separator();

	if (ImGui::BeginTabBar("MaterialTabs")) {
		if (ImGui::BeginTabItem("Properties")) {
			DrawPropertiesTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void Moon::Editor::MaterialEditor::DrawVector2Control(const std::string& label, glm::vec2& values, float resetValue) {
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 100.0f);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// X
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize)) values.x = resetValue;
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f);

	ImGui::PopItemWidth();
	ImGui::SameLine();

	// Y
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f);

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();
}

void Moon::Editor::MaterialEditor::DrawVector3Control(const std::string& label, glm::vec3& values, float resetValue) {
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 100.0f);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// X
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize)) values.x = resetValue;
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// Y
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// Z
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();
}

void Moon::Editor::MaterialEditor::DrawColorControl(const std::string& label, glm::vec3& color) {
	// ProfileFunction
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 100.0f);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();
	ImGui::ColorEdit3("##Color", glm::value_ptr(color), ImGuiColorEditFlags_NoInputs);
	ImGui::Columns(1);
	ImGui::PopID();
}

void Moon::Editor::MaterialEditor::DrawFloatControl(const std::string& label, float& value, float speed, float min, float max) {
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 125.0f);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::DragFloat("##Value", &value, speed, min, max, "%.3f");

	ImGui::Columns(1);
	ImGui::PopID();
}

void Moon::Editor::MaterialEditor::DrawAddPropertyPopup() {
	if (ImGui::BeginPopup("AddPropertyPopup")) {
		static char propName[64] = "";
		static int propType = 0;
		static float floatValue = 0.0f;
		static int intValue = 0;
		static bool boolValue = false;
		static glm::vec2 vec2Value = { 0,0 };
		static glm::vec3 vec3Value = { 0,0,0 };

		ImGui::InputText("Name", propName, sizeof(propName));
		ImGui::Combo("Type", &propType, "Float\0Int\0Bool\0Vector2\0Vector3\0Color\0");

		switch (propType) {
		case 0: ImGui::InputFloat("Default", &floatValue); break;
		case 1: ImGui::InputInt("Default", &intValue); break;
		case 2: ImGui::Checkbox("Default", &boolValue); break;
		case 3: ImGui::InputFloat2("Default", glm::value_ptr(vec2Value)); break;
		case 4: ImGui::InputFloat3("Default", glm::value_ptr(vec3Value)); break;
		}

		if (ImGui::Button("Add") && strlen(propName) > 0) {
			switch (propType) {
			case 0: m_material->SetProperty(propName, floatValue); break;
			case 1: m_material->SetProperty(propName, intValue); break;
			case 2: m_material->SetProperty(propName, boolValue); break;
			case 3: m_material->SetProperty(propName, vec2Value); break;
			case 4: m_material->SetProperty(propName, vec3Value); break;
			}
			m_propertiesDirty = true;
			memset(propName, 0, sizeof(propName));
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Moon::Editor::MaterialEditor::DrawPropertyControl(const std::string& name, Moon::Rendering::MaterialProperty& prop) {
	// ProfileFunction
	
	ImGui::PushID(name.c_str());
	
	std::visit([&](auto&& value) {
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, float>) {
			DrawFloatControl(PrettyName(name), value);
		}
		else if constexpr (std::is_same_v<T, int>) {

		}
		else if constexpr (std::is_same_v<T, bool>) {

		}
		else if constexpr (std::is_same_v<T, glm::vec2>) {
			DrawVector2Control(PrettyName(name), std::get<glm::vec2>(prop.value));
		}
		else if constexpr (std::is_same_v<T, glm::vec3>) {
			if (name.find("Color") != std::string::npos || name.find("color") != std::string::npos) {
				DrawColorControl(PrettyName(name), std::get<glm::vec3>(prop.value));
			}
			else {
				DrawVector3Control(PrettyName(name), std::get<glm::vec3>(prop.value));
			}
		}
		else if constexpr (std::is_same_v<T, std::monostate>) {
			ImGui::TextColored({ 1, 0, 0, 1 }, "%s: Invalid", name.c_str());
		}
		}, prop.value);

	// Single use indicator
	if (prop.singleUse) {
		ImGui::SameLine();
		ImGui::TextDisabled("Single");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Single ue property");
	}

	// Delete button for custom properties
	if (name.find("u_") != 0) {
		ImGui::SameLine();
		if (ImGui::SmallButton("Delete")) {
			m_material->GetProperties().erase(name);
			m_propertiesDirty = true;
		}
	}

	ImGui::PopID();
}

void Moon::Editor::MaterialEditor::DrawPropertiesTab() {
	// ProfileFunction
	
	if (ImGui::Button("Add Property")) {
		ImGui::OpenPopup("AddPropertyPopup");
	}

	DrawAddPropertyPopup();

	auto& properties = m_material->GetProperties();
	if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Draw all properties
		for (auto& [name, prop] : properties) {
			// Skip texture properties (they're handled in textures tab)
			if (std::holds_alternative<Moon::Assets::Texture*>(prop.value))
				continue;

			DrawPropertyControl(name, prop);
			ImGui::Spacing();
		}
	}

	// Texture grid
	if (ImGui::CollapsingHeader("Texture Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
		DrawTextureGrid();
	}

	if (ImGui::CollapsingHeader("Rendering Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Blending
		bool blended = m_material->IsBlendable();
		if (ImGui::Checkbox("Blended", &blended)) {
			m_material->SetBlendable(blended);
		}

		// Cull back face
		bool cullback = m_material->HasBackfaceCulling();
		if (ImGui::Checkbox("Cull Back Face", &cullback)) {
			m_material->SetBackfaceCulling(cullback);
		}

		// Cull front face
		bool cullfront = m_material->HasBackfaceCulling();
		if (ImGui::Checkbox("Cull Front Face", &cullfront)) {
			m_material->SetFrontfaceCulling(cullfront);
		}

		// Color write
		bool colorWrite = m_material->HasColorWriting();
		if (ImGui::Checkbox("Color Writing", &colorWrite)) {
			m_material->SetColorWriting(colorWrite);
		}

		// Depth write/test
		bool depthWrite = m_material->HasDepthWriting();
		if (ImGui::Checkbox("Depth Writing", &depthWrite)) {
			m_material->SetDepthWriting(depthWrite);
		}

		bool depthTest = m_material->HasDepthTest();
		if (ImGui::Checkbox("Depth Test", &depthTest)) {
			m_material->SetDepthTest(depthTest);
		}

		if (ImGui::CollapsingHeader("Material Type")) {
			const char* materialTypes[] = { "STANDARD", "PBR" };
			int currentType = static_cast<int>(m_material->type);
			if (ImGui::Combo("Type", &currentType, materialTypes, IM_ARRAYSIZE(materialTypes))) {
				m_material->type = static_cast<Rendering::MaterialType>(currentType);
			}
		}

		if (ImGui::CollapsingHeader("Presets")) {
			//DrawPresetUI();
		}
	}
}