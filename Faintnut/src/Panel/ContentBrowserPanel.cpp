#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include "Core/String.h"
#include "Core/OS.h"

#include "FileSystem/Directory.h"
#include "AssetManagment/FontAwesome5.h"

#include <imgui.h>

namespace Faint {
	
	/*ContentBrowserPanel::ContentBrowserPanel() {

		m_currentDirectory = FileSystem::RootDirectory;

		m_directoryIcon = CreateRef<Texture>("res/ContentBrowser/DirectoryIcon.png");
		m_fileIcon = CreateRef<Texture>("res/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender() {

		ImGui::Begin("Content Browser");

		//if (m_currentDirectory != std::filesystem::path(s_assetsDirectory)) {
		//	if (ImGui::Button("<-")) {
		//		m_currentDirectory = m_currentDirectory.parent_path();
		//	}
		//}

		Ref<Texture> icon;

		//ImGui::ImageButton("dir", (ImTextureID)icon->GetID(), { 128, 128 }, { 0, 1 }, { 1, 0 });

		ImVec2 avail = ImGui::GetContentRegionAvail();
		int width = avail.x;
		ImVec2 buttonSize = ImVec2(80, 80);
		int amount = (int)(width / 110);
		if (amount <= 0) amount = 1;

		if (m_currentDirectory && m_currentDirectory->Directories.size() > 0)
		{
			for (Ref<Directory>& d : m_currentDirectory->Directories)
			{
				if (d->GetName() == "bin" || d->GetName() == ".vs" || d->GetName() == "obj")
				{
					continue;
				}

				//if (String::Sanitize(d->Name).find(String::Sanitize(m_SearchKeyword)) != std::string::npos)
				//{
				//	if (i + 1 % amount != 0)
				//		ImGui::TableNextColumn();
				//	else
				//		ImGui::TableNextRow();
				//
				//	DrawDirectory(d, i);
				//	i++;
				//}
			}
		}

		//for (auto& directory : std::filesystem::directory_iterator(m_currentDirectory)) {
		//
		//	const auto& dirPath = directory.path();
		//
		//	auto relativePath = std::filesystem::relative(directory.path(), s_assetPath);
		//	auto absolutePath = std::filesystem::absolute(directory.path());
		//	std::string path = directory.path().string();
		//	std::string filename = relativePath.filename().string();
		//	std::string extension = relativePath.extension().string();
		//
		//	ImGui::PushID(relativePath.filename().string().c_str());
		//	Ref<Texture> icon;
		//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		//
		//	//if (extension == ".png" || extension == ".jpg")
		//	//{
		//	//	icon = directory.is_directory() ? m_directoryIcon : m_fileIcon;
		//	//}
		//	//else
		//	//{
		//		icon = directory.is_directory() ? m_directoryIcon : m_fileIcon;
		//	//}
		//
		//	ImGui::ImageButton("dir", (ImTextureID)icon->GetID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
		//
		//	if (ImGui::IsItemHovered()) {
		//		ImGui::SetTooltip(filename.c_str());
		//	}
		//
		//	if (ImGui::BeginDragDropSource()) {
		//
		//		char pathBuffer[256];
		//		std::strncpy(pathBuffer, file->GetAbsolutePath().c_str(), sizeof(pathBuffer));
		//		std::string dragType;
		//
		//		if (fileExtension == ".scene") {
		//			dragType = "_Scene";
		//		}
		//		if (fileExtension == ".cs") {
		//			dragType = "_CSharp";
		//		}
		//		if (fileExtension == ".material") {
		//			dragType = "_Material";
		//		}
		//		if (fileExtension == ".mesh" || fileExtension == ".obj") {
		//			dragType = "_Model";
		//		}
		//		if (fileExtension == ".png" || fileExtension == ".jpg") {
		//			dragType = "_Texture";
		//		}
		//
		//		//ImGui::SetDragDropPayload(dragType.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
		//		ImGui::SetDragDropPayload(dragType.c_str(), (void*)pathBuffer, sizeof(pathBuffer));
		//		ImGui::Text(file->GetName().c_str());
		//		ImGui::EndDragDropSource();
		//	}
		//
		//	ImGui::PopStyleColor();
		//	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		//		if (directory.is_directory())
		//			m_currentDirectory /= directory.path().filename();
		//		if (extension == ".material") {
		//			HZ_CORE_INFO("HELLO");
		//		}
		//	}
		//	ImGui::TextWrapped(relativePath.filename().string().c_str());
		//	ImGui::NextColumn();
		//
		//	ImGui::PopID();
		//}

		ImGui::Columns(1);

		ImGui::End();
	}*/
}