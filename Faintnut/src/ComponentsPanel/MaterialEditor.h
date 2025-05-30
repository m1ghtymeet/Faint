#pragma once
#include "Core/Base.h"
#include "Core/String.h"


#include "Renderer/Types/Material.h"

#include "FileSystem/FileSystem.h"
#include "Util/PlatformUtil.h"
#include "AssetManagment/FontAwesome5.h"
#include "AssetManagment/AssetManager.h"
#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

class MaterialEditor {
public:
	MaterialEditor() = default;
	~MaterialEditor() = default;

	void Draw(Ref<Faint::Material> material) {
		using namespace Faint;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

		std::string materialTitle = material->Path;
		{
			ImGui::PushFont(boldFont);
			ImGui::Text(material->Path.c_str());
			ImGui::PopFont();
		}
		ImGui::SameLine();
		{
			ImGui::PushFont(boldFont);
			if (ImGui::Button(ICON_FA_SAVE)) {

				//if (ResourceManager::IsResourceLoaded(material->id))
				//	ResourceManager::RegisterResource(material);

				if (materialTitle.empty()) {
					std::string saveData = FileDialogs::SaveFile("*.material");
					if (!saveData.empty()) {
						if (!String::EndsWith(saveData, ".material")) {
							saveData += ".material";
						}
					}
					material->Path = FileSystem::AbsoluteToRelative(saveData);
				}
				
				std::string fileData = material->Serialize().dump(4);

				FileSystem::BeginWriteFile(material->Path);
				FileSystem::WriteLine(fileData);
				FileSystem::EndWriteFile();
			}
			ImGui::PopFont();
		}

		bool flagsHeaderOpened;
		{
			ImGui::PushFont(boldFont);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.0f));
			flagsHeaderOpened = ImGui::CollapsingHeader(" FLAGS", ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleVar(2);
			ImGui::PopFont();
		}

		if (flagsHeaderOpened) {
			ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp);
			{
				ImGui::TableSetupColumn("name", 0, 0.3f);
				ImGui::TableSetupColumn("set", 0, 0.6f);
				ImGui::TableSetupColumn("reset", 0, 0.1f);

				ImGui::TableNextColumn();

				ImGui::Text("Unlit");
				ImGui::TableNextColumn();

				ImGui::Text("Emissive");
				ImGui::TableNextColumn();

				//ImGui::DragFloat("##Emissiveness")
			}
			ImGui::EndTable();
		}

		const auto TexturePanelHeight = 100;
		const ImVec2 TexturePanelSize = ImVec2(0, TexturePanelHeight);
		bool AlbedoOpened;
		{
			ImGui::PushFont(boldFont);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.0f));
			AlbedoOpened = ImGui::CollapsingHeader(" ALBEDO", ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleVar(2);
			ImGui::PopFont();
		}

		if (AlbedoOpened) {
			ImGui::BeginChild("##albedo", TexturePanelSize, true, ImGuiWindowFlags_NoScrollbar);
			{
				uint32_t textureID = 0;
				if (material->HasAlbedo()) {
					textureID = material->m_Albedo->GetID();
				}

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
					std::string texture = FileDialogs::OpenFile("*.png | *.jpg");
					if (texture != "") {
						material->SetAlbedo(CreateRef<Texture>(texture));
					}
				}

				if (ImGui::BeginPopupContextWindow()) {
					if (ImGui::MenuItem("Clear Texture")) {
						material->m_Albedo = nullptr;
						material->data.u_HasAlbedo = false;
					}
					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::ColorEdit3("Color", &material->data.m_AlbedoColor.r);
			}
			ImGui::EndChild();
		}

		if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("##normal", TexturePanelSize, true, ImGuiWindowFlags_NoScrollbar);
			{
				uint32_t textureID = 0;
				if (material->HasNormal())
					textureID = material->m_Normal->GetID();

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
				{
					std::string texture = FileDialogs::OpenFile("*.png | *.jpg");
					if (texture != "")
						material->SetNormal(CreateRef<Texture>(texture));
				}

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Clear Texture"))
					{
						material->m_Normal = nullptr;
						material->data.u_HasNormal = false;
					}
					ImGui::EndPopup();
				}
			}
			ImGui::EndChild();
		}

		if (ImGui::CollapsingHeader("Metalness", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("##metalness", TexturePanelSize, true, ImGuiWindowFlags_NoScrollbar);
			{
				uint32_t textureID = 0;
				if (material->HasMetalness())
					textureID = material->m_Metalness->GetID();

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
				{
					std::string texture = FileDialogs::OpenFile("*.png | *.jpg");
					if (texture != "")
						material->SetMetalness(CreateRef<Texture>(texture));
				}

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Clear Texture"))
					{
						material->m_Metalness = nullptr;
						material->data.u_HasMetalness = false;
					}
					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::DragFloat("Value##5", &material->data.u_MetalnessValue, 0.01f, 0.0f, 1.0f);
			}
			ImGui::EndChild();
		}

		if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("##roughness", TexturePanelSize, true, ImGuiWindowFlags_NoScrollbar);
			{
				uint32_t textureID = 0;
				if (material->HasRoughness())
					textureID = material->m_Roughness->GetID();

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
				{
					std::string texture = FileDialogs::OpenFile("*.png | *.jpg");
					if (texture != "")
						material->SetRoughness(CreateRef<Texture>(texture));
				}

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Clear Texture"))
					{
						material->m_Roughness = nullptr;
						material->data.u_HasRoughness = false;
					}
					ImGui::EndPopup();
				}
				ImGui::SameLine();
				ImGui::DragFloat("Value##6", &material->data.u_RoughnessValue, 0.01f, 0.0f, 1.0f);
			}
			ImGui::EndChild();
		}

		ImGui::PopStyleColor();
	}
};