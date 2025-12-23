#include "FileSystemUI.h"
#include <Core/GlobalLocator.h>
#include <Core/OS.h>
#include <Debug/Log.h>
#include <AssetManagment/TextureManager.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <Util/PlatformUtil.h>
#include <Util/ModelTools.h>
#include "../Core/PanelsManager.h"
#include "../Misc/IconGenerator.h"
#include "../Panels/MaterialEditor.h"

#include <FontAwesome5.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <ostream>
#include <fstream>

std::shared_ptr<Moon::Directory> FileBrowser::m_currentDirectory;

FileBrowser::FileBrowser(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_title, p_opened, p_windowSettings)
{
	m_currentDirectory = Moon::FileSystem::GetFileTree();
	if (m_currentDirectory) {
		m_currentDirectory->Scan(true);
	}

	m_defaultFileIcon = Moon::Loaders::TextureLoader::Create("data/editor/icons/Cube.png")->GetTexture().GetID();
}

FileBrowser::~FileBrowser() {
	ClearCache();
}

void FileBrowser::SetFileIcon(Moon::FileType type, uint32_t icon) {
	// Reserved for future implemntation
}

void FileBrowser::ClearCache() {
	// Delete only dynamically created icons
	for (auto& [path, iconID] : m_fileIcons) {
		if (iconID != 0 && iconID != m_defaultFileIcon) {
			glDeleteTextures(1, &iconID);
		}
	}
	m_fileIcons.clear();
	m_loadingQueue.clear();
}

void FileBrowser::MakeDirty(const std::string& filepath) {
	auto iconIt = m_fileIcons.find(filepath);
	if (iconIt != m_fileIcons.end()) {
		if (iconIt->second != 0 && iconIt->second != m_defaultFileIcon) {
			glDeleteTextures(1, &iconIt->second);
		}
		m_fileIcons.erase(iconIt);
	}
	m_loadingQueue.erase(filepath);
}

void FileBrowser::_Draw_Impl() {
	DrawPathNavigation();
	ImGui::Separator();

	// Handle modals
	{ // Draw rename modal if active
		if (m_renameState.isActive) {
			ImGui::OpenPopup("RenameItem");
			m_renameState.isActive = false;
		}

		if (ImGui::BeginPopupModal("RenameItem", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Rename: %s", m_renameState.targetPath.filename().string().c_str());
			ImGui::Spacing();

			bool enterPressed = ImGui::InputText("##NewName", &m_renameState.buffer,
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

			if (!m_renameState.errorMessage.empty()) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				ImGui::TextWrapped("%s", m_renameState.errorMessage.c_str());
				ImGui::PopStyleColor();
			}

			ImGui::Spacing();

			if ((ImGui::Button("Rename", ImVec2(120, 0)) || enterPressed) && !m_renameState.buffer.empty()) {
				std::string errorMsg;
				if (ValidateFileName(m_renameState.buffer, errorMsg)) {
					if (RenameItem(m_renameState.targetPath, m_renameState.buffer)) {
						m_currentDirectory->RefreshIfNeeded();
						ImGui::CloseCurrentPopup();
						m_renameState.errorMessage.clear();
					}
					else {
						m_renameState.errorMessage = "Failed to rename: File may be in use or permission denied";
					}
				}
				else {
					m_renameState.errorMessage = errorMsg;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				m_renameState.errorMessage.clear();
			}

			ImGui::EndPopup();
		}

		// Draw delete confirmation modal
		if (m_deleteState.showConfirmation) {
			ImGui::OpenPopup("ConfirmDelete");
			m_deleteState.showConfirmation = false;
		}
		if (ImGui::BeginPopupModal("ConfirmDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Are you sure you want to delete:");
			ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "%s", m_deleteState.itemName.c_str());

			if (m_deleteState.isDirectory) {
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Warning: This will delete all contents!");
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
			if (ImGui::Button("Delete", ImVec2(120, 0))) {
				if (DeleteItem(m_deleteState.targetPath)) {
					m_currentDirectory->RefreshIfNeeded();
					ImGui::CloseCurrentPopup();
				}
				else {
					HZ_CORE_ERROR("Failed to delete: {}", m_deleteState.targetPath.string());
				}
			}
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	

	ImGui::BeginChild("FileBrowserContent", ImVec2(0, 0), false);

	// Background context menu (right-click on empty space)
	if (ImGui::BeginPopupContextWindow("BackgroundContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		DrawBackgroundContextMenu();
		ImGui::EndPopup();
	}

	// Calculate grid layout
	float windowWidth = ImGui::GetContentRegionAvail().x;
	float itemWidth = m_iconSize + 20.0f; // Icon + padding
	int columns = std::max(1, static_cast<int>(windowWidth / itemWidth));
	float itemHeight = m_iconSize + 40.0f;

	ImGuiListClipper clipper;
	const auto& directories = m_currentDirectory->GetDirectories();
	const auto& files = m_currentDirectory->GetFiles();
	int totalItems = directories.size() + files.size();
	int visibleRows = totalItems > 0 ? (totalItems + columns - 1) / columns : 0;

	clipper.Begin(visibleRows, itemHeight);

	while (clipper.Step()) {
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
			for (int col = 0; col < columns; ++col) {
				int itemIdx = row * columns + col;

				if (col > 0) ImGui::SameLine();

				// Directory
				if (itemIdx < (int)directories.size()) {
					if (DrawDirectoryItem(directories[itemIdx])) {
						m_currentDirectory = directories[itemIdx];
					}
				}
				// File
				else if (itemIdx < (int)directories.size() + (int)files.size()) {
					int fileIdx = itemIdx - directories.size();
					DrawFileItem(files[fileIdx]);
				}
				else {
					break;
				}
			}
		}
	}

	clipper.End();
	ImGui::EndChild();
}

void FileBrowser::DrawPathNavigation() {
	auto pathComponents = m_currentDirectory->GetPathFromRoot();

	// Get root directory to check if we can go back
	auto rootDir = Moon::FileSystem::GetFileTree();

	// Check if we're at root - use pointer comparison
	bool isAtRoot = (m_currentDirectory.get() == rootDir.get());

	// Also check if parent exists and is valid
	auto parent = m_currentDirectory->GetParent().lock();
	bool hasValidParent = (parent != nullptr);

	// Disable back button only if we're truly at root
	ImGui::BeginDisabled(isAtRoot || !hasValidParent);
	if (ImGui::Button(ICON_FA_ARROW_LEFT " Back")) {
		if (parent && !isAtRoot) {
			m_currentDirectory = parent;
		}
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::Text("Path:");
	ImGui::SameLine();

	// Draw breadcrumb path
	for (size_t i = 0; i < pathComponents.size(); ++i) {
		if (i > 0) {
			ImGui::SameLine();
			ImGui::TextDisabled("/");
			ImGui::SameLine();
		}

		ImGui::PushID(static_cast<int>(i));
		if (ImGui::Button(pathComponents[i]->GetName().c_str())) {
			m_currentDirectory = pathComponents[i];
		}
		ImGui::PopID();
	}
}

bool FileBrowser::DrawDirectoryItem(const std::shared_ptr<Moon::Directory>& dir) {
	bool clicked = false;
	ImGui::BeginGroup();

	ImVec2 padding = { 8, 8 };
	ImVec2 iconSize = { 100, 100 };

	ImVec2 min = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(("DirBtn_" + dir->GetName()).c_str(), { iconSize.x + padding.x * 2, iconSize.y + padding.y * 2 });
	ImVec2 max = ImGui::GetItemRectMax();

	// Check for double-click on icon
	if (ImGui::IsItemHovered()) {
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(100, 100, 255, 50), 5.0f);
		if (ImGui::IsMouseDoubleClicked(0))
			clicked = true;
	}
	// Right-click context menu on icon
	if (ImGui::BeginPopupContextItem(("DirCtx_" + dir->GetName()).c_str())) {
		DrawItemContextMenu(dir);
		ImGui::EndPopup();
	}
	// Drag & Drop source
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("_DIR", dir.get(), sizeof(Moon::Directory));
		ImGui::Text("%s", dir->GetName().c_str());
		ImGui::EndDragDropSource();
	}
	// Icon
	ImGui::SetCursorScreenPos({ min.x + padding.x, min.y + padding.y });
	ImGui::Image((void*)FTSERVICE(Moon::AssetManagment::TextureManager)
		.CreateResource("data/editor/icons/FolderIcon.png")->GetTexture().GetID(),
		iconSize, { 0, 1 }, { 1, 0 });
	// Text
	float textWidth = m_iconSize + 20.0f;
	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + textWidth);

	std::string fileName = dir->GetName();
	float textSize = ImGui::CalcTextSize(fileName.c_str()).x;
	if (textSize < textWidth) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (textWidth - textSize) * 0.5f);
	}

	ImGui::TextWrapped("%s", fileName.c_str());
	ImGui::PopTextWrapPos();

	ImGui::EndGroup();
	return clicked;
}

void FileBrowser::DrawFileItem(const std::shared_ptr<Moon::File>& file) {
	ImGui::BeginGroup();

	ImVec2 padding = { 8, 8 };
	ImVec2 iconSize = { 100, 100 };

	ImVec2 min = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(("FileBtn_" + file->GetName()).c_str(), { iconSize.x + padding.x * 2, iconSize.y + padding.y * 2 });
	ImVec2 max = ImGui::GetItemRectMax();

	if (ImGui::IsItemHovered()) {
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(100, 255, 100, 50), 5.0f);
		if (ImGui::IsMouseDoubleClicked(0)) {
			Moon::OS::OpenIn(file->GetAbsolutePath().string());
		}
	}

	// Drag & Drop source
	std::string payloadType = GetDragDropType(file);
	if (!payloadType.empty() && ImGui::BeginDragDropSource()) {
		std::string filepath = file->GetAbsolutePath().string();
		ImGui::SetDragDropPayload(payloadType.c_str(), filepath.c_str(), filepath.size() + 1);
		ImGui::Text("%s", file->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	// Right-click context menu on icon
	if (ImGui::BeginPopupContextItem(("FileCtx_" + file->GetName()).c_str())) {
		DrawItemContextMenu(file);
		ImGui::EndPopup();
	}

	// Icon
	ImGui::SetCursorScreenPos({ min.x + padding.x, min.y + padding.y });
	ImTextureID icon = (ImTextureID)GetFileIcon(file);
	if (icon) ImGui::Image(icon, iconSize, { 0, 1 }, { 1, 0 });

	float textWidth = m_iconSize + 20.0f;
	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + textWidth);

	std::string fileName = file->GetName();
	float textSize = ImGui::CalcTextSize(fileName.c_str()).x;
	if (textSize < textWidth) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (textWidth - textSize) * 0.5f);
	}

	ImGui::TextWrapped("%s", fileName.c_str());
	ImGui::PopTextWrapPos();

	ImGui::EndGroup();
}

void FileBrowser::DrawItemContextMenu(const std::shared_ptr<Moon::Directory>& dir) {
	ImGui::TextDisabled("Directory: %s", dir->GetName().c_str());
	ImGui::Separator();
	if (ImGui::MenuItem("Open in Explorer")) {
		Moon::OS::ShowInFileExplorer(dir->GetFullPath().string());
	}
	if (ImGui::MenuItem("Rename")) {
		m_renameState.buffer = dir->GetName();
		m_renameState.isActive = true;
		m_renameState.isDirectory = true;
		m_renameState.targetPath = dir->GetFullPath();
		m_renameState.errorMessage.clear();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Copy Path")) {
		Moon::OS::CopyToClipboard(dir->GetFullPath().string());
	}
	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
	if (ImGui::MenuItem("Delete")) {
		m_deleteState.showConfirmation = true;
		m_deleteState.targetPath = dir->GetFullPath();
		m_deleteState.itemName = dir->GetName();
		m_deleteState.isDirectory = true;
	}
	ImGui::PopStyleColor();
}

void FileBrowser::DrawItemContextMenu(const std::shared_ptr<Moon::File>& file) {
	ImGui::TextDisabled("File: %s", file->GetName().c_str());
	ImGui::Separator();
	if (ImGui::MenuItem("Open")) {
		Moon::OS::OpenIn(file->GetAbsolutePath().string());
	}
	if (ImGui::MenuItem("Edit", "", false, file->GetFileType() == Moon::FileType::MATERIAL)) {
		auto& pm = FTSERVICE(PanelsManager).GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor");
		pm.Open();
		pm.Focus();
		pm.SetMaterial(*Moon::Loaders::MaterialLoader::Create(file->GetAbsolutePath().string(), true), false);
	}
	if (ImGui::MenuItem("Rename")) {
		m_renameState.buffer = file->GetName();
		m_renameState.isActive = true;
		m_renameState.isDirectory = false;
		m_renameState.targetPath = file->GetFullPath();
		m_renameState.errorMessage.clear();
	}
	{
		std::string ext = file->GetExtension();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		bool isModel = (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb");
		if (isModel) {
			ImGui::Separator();
			if (ImGui::MenuItem("Generate Materials")) {
				Moon::ModelData modelData = Moon::AssimpImporter::ImportFbx(file->GetAbsolutePath().string());
				if (!modelData.materialNames.empty()) {
					std::filesystem::path basePath = file->GetFullPath().parent_path();
					std::filesystem::path templatePath = "data/engine/materials/default.ftmat";
					for (const auto& matName : modelData.materialNames) {
						if (matName.empty()) continue;
						std::filesystem::path matPath = basePath / (matName + ".mnmat");

						int counter = 1;
						std::filesystem::path uniquePath = matPath;
						while (std::filesystem::exists(uniquePath)) {
							uniquePath = basePath / (matName + "_" + std::to_string(counter++) + ".mnmat");
						}

						std::ifstream in(templatePath);
						if (in) {
							std::ofstream out(uniquePath);
							out << in.rdbuf();  // Copy the full template
							out.close();
						}
						else {
							std::ofstream out(uniquePath);
							out << "{}";
							out.close();
						}
						in.close();
						HZ_CORE_INFO("Material created: {}", uniquePath.string());
					}
					//m_currentDirectory->RefreshIfNeeded();
				}
				else {
					HZ_CORE_WARN("No materials found in model: {}", file->GetName());
				}
			}
			if (ImGui::MenuItem("Split Meshes")) {
				std::filesystem::path basePath = file->GetFullPath().parent_path();
				std::string baseName = file->GetFullPath().stem().string();
				Moon::ModelData fullModel = Moon::AssimpImporter::ImportFbx(file->GetAbsolutePath().string());
				for (size_t i = 0; i < fullModel.meshCount; i++) {
					const auto& srcMesh = fullModel.meshes[i];
					Moon::ModelData singleMeshModel;
					singleMeshModel.name = srcMesh.name.empty() ? (baseName + "_Mesh" + std::to_string(i)) : srcMesh.name;
					singleMeshModel.meshCount = 1;
					singleMeshModel.meshes.push_back(srcMesh);
					if (srcMesh.materialIndex < fullModel.materialNames.size()) {
						singleMeshModel.materialNames.push_back(fullModel.materialNames[srcMesh.materialIndex]);
					}

					std::string meshFileName = singleMeshModel.name + ".mnmodel";
					std::filesystem::path meshPath = basePath / meshFileName;

					int counter = 1;
					std::filesystem::path uniquePath = meshPath;
					while (std::filesystem::exists(uniquePath)) {
						meshFileName = singleMeshModel.name + "_" + std::to_string(counter++) + ".mnmodel";
						uniquePath = basePath / meshFileName;
					}

					Moon::ModelTools::ExportModel(singleMeshModel, uniquePath.string());
					HZ_CORE_INFO("Explored split mesh: {}", uniquePath.string());
				}
				//m_currentDirectory->RefreshIfNeeded();
			}
		}
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Copy Path")) {
		Moon::OS::CopyToClipboard(file->GetAbsolutePath().string());
	}
	if (ImGui::MenuItem("Copy Relative Path")) {
		Moon::OS::CopyToClipboard(file->GetRelativePath());
	}
	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
	if (ImGui::MenuItem("Delete")) {
		m_deleteState.showConfirmation = true;
		m_deleteState.targetPath = file->GetFullPath();
		m_deleteState.itemName = file->GetName();
		m_deleteState.isDirectory = false;
	}
	ImGui::PopStyleColor();
}

void FileBrowser::DrawBackgroundContextMenu() {
	ImGui::TextDisabled("Current Folder: %s", m_currentDirectory->GetName().c_str());
	ImGui::Separator();

	if (ImGui::BeginMenu("New...")) {
		if (ImGui::MenuItem("Folder")) {
			std::filesystem::path newFolderPath = m_currentDirectory->GetFullPath() / "New Folder";
			int counter = 1;
			while (std::filesystem::exists(newFolderPath)) {
				newFolderPath = m_currentDirectory->GetFullPath() / ("New Folder " + std::to_string(counter++));
			}
			std::filesystem::create_directory(newFolderPath);
			m_currentDirectory->RefreshIfNeeded();
		}
		if (ImGui::MenuItem("Material")) {
			std::filesystem::path templatePath = "data/engine/materials/default.ftmat";
			std::filesystem::path newMatPath = m_currentDirectory->GetFullPath() / "NewMaterial.mnmat";
			int counter = 1;
			while (std::filesystem::exists(newMatPath)) {
				newMatPath = m_currentDirectory->GetFullPath() / ("NewMaterial " + std::to_string(counter++) + ".mnmat");
			}

			std::ifstream in(templatePath);
			if (in) {
				std::ofstream out(newMatPath);
				out << in.rdbuf();  // Copy the full template
			}
			else {
				std::ofstream out(newMatPath);
				out << "{}";
			}
			m_currentDirectory->RefreshIfNeeded();
		}
		if (ImGui::MenuItem("Script")) {
			std::filesystem::path newScriptPath = m_currentDirectory->GetFullPath() / "NewScript.lua";
			int counter = 1;
			while (std::filesystem::exists(newScriptPath)) {
				newScriptPath = m_currentDirectory->GetFullPath() / ("NewScript " + std::to_string(counter++) + ".lua");
			}

			std::ofstream out(newScriptPath);
			out <<
				"-- Lua Script\n"
				"local NewScript = {}\n"
				"function NewScript:Start()\n"
				"    print(\"Script started\")\n"
				"end\n\n"
				"function NewScript:Update(dt)\n"
				"end\n"
				"return NewScript";
			m_currentDirectory->RefreshIfNeeded();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Import Asset")) {
		std::string srcPath = Moon::FileDialogs::OpenFile(
			"3D Models (.obj,.fbx,.gltf,.glb)\0*.obj;*.fbx;*.gltf;*.glb\0"
			"Texture (.png,.jpeg,.jpg,.tga,.dds,.exr)\0*.png;*.jpeg;*.jpg;*.tga;*.dds;*.exr\0"
			"Sound (.mp3,.ogg,.wav)\0*.mp3;*.ogg;*.wav\0"
			"Shader (.mnshader,.vert,.frag)\0*.mnshader;*.ftshader;*.vert;*.frag\0"
			"All Files\0*.*\0"
		);
		if (!srcPath.empty()) {
			try {
				std::string dstPath = m_currentDirectory->GetFullPath().string();
				std::filesystem::copy(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing);
				HZ_CORE_INFO("Asset \"" + dstPath + std::filesystem::path(srcPath).stem().string() + "\" imported");
			}
			catch (const std::exception& e) {
				HZ_CORE_ERROR("Failed to import asset: {}", e.what());
			}
		}
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Refresh")) {
		m_currentDirectory->Scan(true);
	}
}

uint32_t FileBrowser::GetFileIcon(const std::shared_ptr<Moon::File>& file) {
	const std::string& filepath = file->GetAbsolutePath().string();
	auto it = m_fileIcons.find(filepath);
	if (it != m_fileIcons.end() && it->second) {
		return it->second;
	}
	if (m_loadingQueue.find(filepath) != m_loadingQueue.end()) {
		return m_placeholderIcon;
	}
	Moon::FileType fileType = file->GetFileType();
	if (Moon::FileSystem::FileExists(file->GetAbsolutePath().string(), true)) {
		LoadIconAsync(file);
	}
	return 0;
}

void FileBrowser::LoadIconAsync(const std::shared_ptr<Moon::File>& file) {
	const std::string& filePath = file->GetAbsolutePath().string();

	if (m_loadingQueue.find(filePath) != m_loadingQueue.end()) {
		return;
	}
	if (m_loadingQueue.size() >= MAX_CONCURRENT_LOADS) {
		return;
	}

	m_loadingQueue.insert(filePath);

	//std::thread([this, file, filePath]() {
		try {
			uint32_t icon = m_defaultFileIcon;

			if (file->GetFileType() == Moon::FileType::TEXTURE) {
				icon = Moon::Loaders::TextureLoader::Create(filePath)
					->GetTexture().GetID();
			}
			else if (file->GetFileType() == Moon::FileType::MATERIAL) {
				IconGenerator generator;
				auto material = Moon::Loaders::MaterialLoader::Create(filePath, true);
				if (material) {
					icon = generator.GenerateAndReturn(*material)->GetID();
				}
			}
			m_fileIcons[filePath] = icon;
		}
		catch (const std::exception& e) {
			m_fileIcons[filePath] = m_defaultFileIcon;
		}
		m_loadingQueue.erase(filePath);

	//	}).detach();
}

std::string FileBrowser::GetDragDropType(const std::shared_ptr<Moon::File>& file) {
	std::string ext = file->GetExtension();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".dds")
		return "_TEXTURE";
	if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb" || ext == ".mnmodel")
		return "_MODEL";
	if (ext == ".ttf" || ext == ".otf" || ext == ".atlas.png") return "_FONT";
	if (ext == ".mnmat" || ext == ".ftmat") return "_MATERIAL";
	if (ext == ".mnshader" || ext == ".ftshader") return "_SHADER";
	if (ext == ".scene" || ext == ".mnscene") return "_SCENE";
	if (ext == ".prefab") return "_PREFAB";
	if (ext == ".lua") return "_SCRIPT";
	return "";
}

bool FileBrowser::ValidateFileName(const std::string& name, std::string& errorMsg) {
	if (name.empty()) {
		errorMsg = "Name cannot be empty";
		return false;
	}

	// Invalid characetrs for Windows and Unix
	const std::string invallidChars = "<>:\"/\\|?*";
	for (char c : invallidChars) {
		if (name.find(c) != std::string::npos) {
			errorMsg = "Name contains invalid characters: " + std::string(1, c);
			return false;
		}
	}

	// Reserved names on Windows
	const std::vector<std::string> reservedNames = {
		"CON", "PRN", "AUX", "NUL",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
	};
	std::string upperName = name;
	std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
	for (const auto& reserved : reservedNames) {
		if (upperName == reserved || upperName.find(reserved + ".") == 0) {
			errorMsg = "Name is reserved by the system";
			return false;
		}
	}

	if (name[0] == ' ' || name[name.length() - 1] == ' ') {
		errorMsg = "Name cannot start or end with a space";
		return false;
	}

	return true;
}

bool FileBrowser::RenameItem(const std::filesystem::path& oldPath, const std::string& newName) {
	try {
		std::filesystem::path newPath = oldPath.parent_path() / newName;
		if (Moon::FileSystem::FileExists(newPath.string(), true)) {
			HZ_CORE_WARN("Cannot rename: Target already exists");
			return false;
		}

		// Invalidate cache before rename
		MakeDirty(oldPath.string());

		std::filesystem::rename(oldPath, newPath);
		HZ_CORE_INFO("Renamed: {} -> {}", oldPath.string(), newPath.string());
		return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		HZ_CORE_ERROR("Rename failed: {}", e.what());
		return false;
	}
}

bool FileBrowser::DeleteItem(const std::filesystem::path& filepath) {
	try {
		// Invalidate cache before delete
		MakeDirty(filepath.string());

		// If it's directory, recurisvely invalidate all file icons
		if (std::filesystem::is_directory(filepath)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(filepath)) {
				if (entry.is_regular_file())
					MakeDirty(entry.path().string());
			}
		}

		std::filesystem::remove_all(filepath);
		HZ_CORE_INFO("Deleted: {}", filepath.string());
		return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		HZ_CORE_ERROR("Delete failed: {}", e.what());
		return false;
	}
}
