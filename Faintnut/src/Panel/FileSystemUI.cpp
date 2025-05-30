#include "FileSystemUI.h"
#include "UI/PopupHelper.h"
#include "UI/ImUI.h"

#include "Core/OS.h"
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
#include "Renderer/Manager/TextureManager.h"
#include "../Misc/ThumbnailManager.h"

#include "AssetManagment/FontAwesome5.h"

#include <Hazel.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace Faint
{
	Ref<Directory> FileSystemUI::m_CurrentDirectory;
	
	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min.x = window->DC.CursorPos.x + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1)).x;
		bb.Min.y = window->DC.CursorPos.y + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1)).y;
		bb.Max.x = bb.Min.x + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f).x;
		bb.Max.y = bb.Min.y + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f).y;
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

    static float sz1 = 300;
    static float sz2 = 300;
    void FileSystemUI::Draw()
    {
        if (ImGui::Begin("File Browser"))
        {
            Ref<Directory> rootDirectory = FileSystem::GetFileTree();
            if (!rootDirectory)
                return;

            ImVec2 avail = ImGui::GetContentRegionAvail();
            Splitter(true, 4.0f, &sz1, &sz2, 100, 8, avail.y);

            ImVec4* colors = ImGui::GetStyle().Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 8);
            if (ImGui::BeginChild("Tree browser", ImVec2(sz1, avail.y), true))
            {
                ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;

                bool is_selected = m_CurrentDirectory == FileSystem::RootDirectory;
                if (is_selected)
                    base_flags |= ImGuiTreeNodeFlags_Selected;

                std::string icon = ICON_FA_FOLDER;
                //ImGui::PushFont(FontManager::GetFont(Bold));
                bool open = ImGui::TreeNodeEx("PROJECT", base_flags);
                if (ImGui::IsItemClicked())
                {
                    m_CurrentDirectory = FileSystem::RootDirectory;
                }
                //ImGui::PopFont();

                for (auto& d : rootDirectory->Directories)
                    EditorInterfaceDrawFiletree(d);

                ImGui::TreePop();

            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::SameLine();

            std::vector<Ref<Directory>> paths = std::vector<Ref<Directory>>();

            Ref<Directory> currentParent = m_CurrentDirectory;
            paths.push_back(m_CurrentDirectory);

            while (currentParent != nullptr)
            {
                paths.push_back(currentParent);
                currentParent = currentParent->Parent;
            }

            avail = ImGui::GetContentRegionAvail();
            if (ImGui::BeginChild("Wrapper", avail))
            {
                avail.y = 30;
                if (ImGui::BeginChild("Path", avail, true))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 4 });
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                    const auto buttonSize = ImVec2(26, 26);
                    std::string refreshIcon = ICON_FA_SYNC_ALT;
                    if (ImGui::Button((refreshIcon).c_str(), buttonSize))
                    {
                        FileSystem::Scan();
                    }

                    ImGui::SameLine();

                    const auto cursorStart = ImGui::GetCursorPosX();
                    if (ImGui::Button((std::string(ICON_FA_ANGLE_LEFT)).c_str(), buttonSize))
                    {
                        if (m_CurrentDirectory != FileSystem::RootDirectory)
                        {
                            m_CurrentDirectory = m_CurrentDirectory->Parent;
                        }
                    }

                    ImGui::SameLine();

                    const auto cursorEnd = ImGui::GetCursorPosX();
                    const auto buttonWidth = cursorEnd - cursorStart;
                    if (ImGui::Button((std::string(ICON_FA_ANGLE_RIGHT)).c_str(), buttonSize))
                    {
                        //if (Editor->Selection.Type == EditorSelectionType::Directory)
                        //{
                        m_CurrentDirectory = m_EditorDir;
                        //}
                    }

                    const uint32_t numButtonAfterPathBrowser = 2;
                    const uint32_t searchBarSize = 6;
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                    ImGui::BeginChild("pathBrowser", ImVec2((ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth * searchBarSize)) - 4.0, 24));
                    for (int i = paths.size() - 1; i > 0; i--)
                    {
                        if (i != paths.size())
                            ImGui::SameLine();

                        std::string pathLabel;
                        if (i == paths.size() - 1)
                        {
                            pathLabel = "Project files";
                        }
                        else
                        {
                            pathLabel = paths[i]->Name;
                        }

                        if (ImGui::Button(pathLabel.c_str()))
                        {
                            m_CurrentDirectory = paths[i];
                        }

                        ImGui::SameLine();
                        ImGui::Text("/");
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar();

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();

                    ImGui::SameLine();

                    ImGui::BeginChild("searchBar", ImVec2(ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth), 24));
                    char buffer[256];
                    memset(buffer, 0, sizeof(buffer));
                    std::strncpy(buffer, m_SearchKeyword.c_str(), sizeof(buffer));
                    if (ImGui::InputTextEx("##Search", "Asset search & filter ..", buffer, sizeof(buffer), ImVec2(ImGui::GetContentRegionAvail().x, 24), ImGuiInputTextFlags_EscapeClearsAll))
                    {
                        m_SearchKeyword = std::string(buffer);
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    if (ImGui::Button((std::string(ICON_FA_FOLDER_OPEN)).c_str(), buttonSize))
                    {
                        OS::OpenIn(m_CurrentDirectory->FullPath);
                    }

                    ImGui::PopStyleColor(); // Button color

                    ImGui::SameLine();
                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetCursorPosY()), IM_COL32(255, 0, 0, 255), 1.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                avail = ImGui::GetContentRegionAvail();

                bool child = ImGui::BeginChild("Content", avail);
                ImGui::PopStyleVar();
                ImGui::SameLine();
                if (child)
                {
                    int width = avail.x;
                    ImVec2 buttonSize = ImVec2(80, 80);
                    int amount = (int)(width / 110);
                    if (amount <= 0) amount = 1;

                    int i = 1; // current amount of item per row.
                    if (ImGui::BeginTable("ssss", amount))
                    {
                        // Button to go up a level.
                        //if (m_CurrentDirectory && m_CurrentDirectory != FileSystem::RootDirectory && m_CurrentDirectory->Parent)
                        //{
                        //    ImGui::TableNextColumn();
                        //    if (ImGui::Button("..", buttonSize))
                        //        m_CurrentDirectory = m_CurrentDirectory->Parent;
                        //    i++;
                        //}

                        if (m_CurrentDirectory && m_CurrentDirectory->Directories.size() > 0)
                        {
                            for (Ref<Directory>& d : m_CurrentDirectory->Directories)
                            {
                                if (d->GetName() == "bin" || d->GetName() == ".vs" || d->GetName() == "obj")
                                {
                                    continue;
                                }

                                if (String::Sanitize(d->Name).find(String::Sanitize(m_SearchKeyword)) != std::string::npos)
                                {
                                    if (i + 1 % amount != 0)
                                        ImGui::TableNextColumn();
                                    else
                                        ImGui::TableNextRow();

                                    DrawDirectory(d, i);
                                    i++;
                                }
                            }
                        }

                        if (m_CurrentDirectory && m_CurrentDirectory->Files.size() > 0)
                        {
                            for (auto& f : m_CurrentDirectory->Files)
                            {
                                if (m_SearchKeyword.empty() || f->GetName().find(String::Sanitize(m_SearchKeyword)) != std::string::npos)
                                {
                                    if (f->GetFileType() == FileType::Unknown || f->GetFileType() == FileType::Assembly)
                                    {
                                        continue;
                                    }

                                    if (i + 1 % amount != 0 || i == 1)
                                    {
                                        ImGui::TableNextColumn();
                                    }
                                    else
                                    {
                                        ImGui::TableNextRow();
                                    }

                                    DrawFile(f, i);
                                    i++;
                                }
                            }
                        }

                        //DrawContextMenu();
                        m_HasClickedOnFile = false;

                        ImGui::EndTable();
                    }
                }
                ImGui::EndChild();
            }

            ThumbnailManager::Get().OnEndFrame();

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void FileSystemUI::EditorInterfaceDrawFiletree(Ref<Directory> dir)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
        //if (is_selected) 

        if (m_CurrentDirectory == dir)
            base_flags |= ImGuiTreeNodeFlags_Selected;
        if (dir->Directories.size() <= 0)
            base_flags |= ImGuiTreeNodeFlags_Leaf;

        std::string icon = ICON_FA_FOLDER;
        bool open = ImGui::TreeNodeEx((icon + "  " + dir->Name.c_str()).c_str(), base_flags);

        if (ImGui::IsItemClicked())
            m_CurrentDirectory = dir;

        if (open)
        {
            if (dir->Directories.size() > 0)
                for (auto& d : dir->Directories)
                    EditorInterfaceDrawFiletree(d);
            ImGui::TreePop();
        }
    }

    void FileSystemUI::DrawFileTree(Ref<Directory> directory)
    {
    }

    void FileSystemUI::DrawDirectory(Ref<Directory> directory, uint32_t drawId)
    {
        HZ_PROFILE_FUNCTION();

        //ImGui::PushFont(FontManager::GetFont(Icons));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        const char* icon = ICON_FA_FOLDER;
        const std::string id = std::string("##") + directory->Name;

        ImVec2 prevCursor = ImGui::GetCursorPos();
        ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
        const bool selected = ImGui::Selectable(id.c_str(), /*Editor->Selection.Directory == directory*/true, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));
        const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
        {
            ImGui::OpenPopup(hoverMenuId.c_str());
            m_HasClickedOnFile = true;
        }

        const std::string renameId = "Rename" + std::string("##") + hoverMenuId;
        bool shouldRename = false;

        const std::string deleteId = "Delete" + std::string("##") + hoverMenuId;
        bool shouldDelete = false;

        if (selected)
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                m_CurrentDirectory = directory;
            }

            m_EditorDir = directory;
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(directory->Name.c_str());


        ImGui::SetCursorPos(prevCursor);
        ImGui::Image((ImTextureID)TextureManager::Get()->GetTexture("data/editor/icons/DirectoryIcon.png")->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));

        auto imguiStyle = ImGui::GetStyle();

        ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
        ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.CellPadding.y / 2.0f);
        ImU32 rectColor = IM_COL32(255, 255, 255, 16);
        ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);
        std::string visibleName = directory->Name;
        const uint32_t MAX_CHAR_NAME = 34;
        if (directory->Name.size() > MAX_CHAR_NAME)
        {
            visibleName = std::string(directory->Name.begin(), directory->Name.begin() + MAX_CHAR_NAME - 3) + "...";
        }

        ImGui::TextWrapped(visibleName.c_str());

        ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
        ImGui::TextColored({ 1, 1, 1, 0.5f }, "Folder");

        ImGui::PopStyleVar();


        if (ImGui::BeginPopup(hoverMenuId.c_str()))
        {
            if (ImGui::MenuItem("Open"))
            {
                m_CurrentDirectory = directory;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Copy"))
            {
                if (ImGui::MenuItem("Full Path"))
                {
                    OS::CopyToClipboard(directory->FullPath);
                }

                if (ImGui::MenuItem("Directory Name"))
                {
                    OS::CopyToClipboard(String::Split(directory->Name, '/')[0]);
                }

                ImGui::EndPopup();
            }

            if (ImGui::MenuItem("Delete"))
            {
                shouldDelete = true;
            }

            if (ImGui::MenuItem("Rename"))
            {
                shouldRename = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Show in File Explorer"))
            {
                OS::OpenIn(directory->FullPath);
            }

            ImGui::EndPopup();
        }

        // Rename Popup
        if (shouldRename)
        {
            renameTempValue = directory->Name;
            PopupHelper::OpenPopup(renameId);
        }

        if (PopupHelper::TextDialog(renameId, renameTempValue))
        {
            if (OS::RenameDirectory(directory, renameTempValue) != 0)
            {
                //Logger::Log("Cannot rename directory: " + renameTempValue, "editor", CRITICAL);
                HZ_CORE_ERROR("Cannot rename directory: {0}", renameTempValue);
            }
            //RefreshFileBrowser();
            FileSystem::Scan();
            renameTempValue = "";
        }

        // Delete Popup
        if (shouldDelete)
        {
            PopupHelper::OpenPopup(deleteId);
        }

        if (PopupHelper::ConfirmationDialog(deleteId, " Are you sure you want to delete the folder and all its children?\n This action cannot be undone, and all data within the folder \n will be permanently lost."))
        {
            if (FileSystem::DeleteFolder(directory->FullPath) != 0)
            {
                //Logger::Log("Failed to remove directory: " + directory->Name, "editor", CRITICAL);
                HZ_CORE_ERROR("Failed to remove directory: {0}", directory->Name);
            }
            //RefreshFileBrowser();
            FileSystem::Scan();
        }

        //ImGui::PopFont();
    }

    void FileSystemUI::DrawFile(Ref<File> file, uint32_t drawId)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });
        std::string fileExtension = file->GetExtension();

        ImVec2 prevCursor = ImGui::GetCursorPos();
        ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
        std::string id = std::string("##") + file->GetAbsolutePath();
        const bool selected = ImGui::Selectable(id.c_str(), true, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));

        const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
        {
            ImGui::OpenPopup(hoverMenuId.c_str());
            m_HasClickedOnFile = true;
        }

        bool shouldOpenScene = false;
        if (selected) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                
            }

            m_EditorFile = file;
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(file->GetName().c_str());

        if (ImGui::BeginDragDropSource()) {

            char pathBuffer[256];
            std::strncpy(pathBuffer, file->GetAbsolutePath().c_str(), sizeof(pathBuffer));
            std::string dragType;

            if (fileExtension == ".scene") {
                dragType = "_Scene";
            }
            if (fileExtension == ".cs") {
                dragType = "_CSharp";
            }
            if (fileExtension == ".wren") {
                dragType = "_Script";
            }
            if (fileExtension == ".material") {
                dragType = "_Material";
            }
            if (fileExtension == ".mesh" || fileExtension == ".obj") {
                dragType = "_Model";
            }
            if (fileExtension == ".png" || fileExtension == ".jpg") {
                dragType = "_Texture";
            }
            if (fileExtension == ".wav" || fileExtension == ".mp3") {
                dragType = "_Audio";
            }

            //ImGui::SetDragDropPayload(dragType.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
            ImGui::SetDragDropPayload(dragType.c_str(), (void*)pathBuffer, sizeof(pathBuffer));
            ImGui::Text(file->GetName().c_str());
            ImGui::EndDragDropSource();
        }

        Ref<Texture> textureImage = TextureManager::Get()->GetTexture("data/editor/icons/FileIcon.png");

        const auto fileType = file->GetFileType();
        if (fileType == FileType::Material) {
        
            auto image = ThumbnailManager::Get().GetThumbnail(file->GetAbsolutePath());
            if (image)
                textureImage = image;
        }
        if (fileType == FileType::Image) {
            const std::string path = file->GetAbsolutePath();
            textureImage = TextureManager::Get()->GetTexture(path);
        }
        else if (fileType == FileType::Prefab) {
            auto image = ThumbnailManager::Get().GetThumbnail(file->GetAbsolutePath());
            if (image)
            {
                textureImage = image;
            }
        }

        ImGui::SetCursorPos(prevCursor);
        ImGui::Image(reinterpret_cast<ImTextureID>(textureImage->GetID()), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::PopStyleVar(2);

        auto& imguiStyle = ImGui::GetStyle();

        ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
        ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.CellPadding.y / 2.0f);
        ImU32 rectColor = IM_COL32(255, 255, 255, 16);
        ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);
    
        ImU32 rectColor2 = UI::PrimaryCol;
        Color fileTypeColor = /*GetColorByFileType(file->GetFileType())*/{ 1, 1, 1, 1 };
        ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 101) + offsetEnd, IM_COL32(fileTypeColor.r * 255.f, fileTypeColor.g * 255.f, fileTypeColor.b * 255.f, fileTypeColor.a * 255.f), 0.0f);
    
        std::string visibleName = file->GetName();
        const uint32_t MAX_CHAR_NAME = 32;
        if (file->GetName().size() >= MAX_CHAR_NAME)
        {
            visibleName = std::string(visibleName.begin(), visibleName.begin() + MAX_CHAR_NAME - 3) + "...";
        }

        ImGui::TextWrapped(visibleName.c_str());

        ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
        ImGui::TextColored({ 1, 1, 1, 0.5f }, file->GetFileTypeAsString().c_str());

        if (file->GetFileType() != FileType::Prefab) {
            ThumbnailManager::Get().MarkThumbnailAsDirty(file->GetAbsolutePath());
        }
    }
}
