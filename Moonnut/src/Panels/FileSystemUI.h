#pragma once
#include "../UI/Panels/PanelWindow.h"
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <unordered_set>

class FileBrowser : public PanelWindow {
public:
    enum class ViewMode { Grid, List };
    enum class SortMode { NameAsc, NameDesc, Type, DateModified };

    FileBrowser(
        const std::string& p_title = "Content Browser",
        bool p_opened = true,
        const PanelWindowSettings& p_windowSettings = {}
    );
    ~FileBrowser();

    static std::shared_ptr<Moon::Directory> m_currentDirectory;

    void SetFolderIcon(uint32_t icon) { m_folderIcon = icon; }
    void SetFileIcon(Moon::FileType type, uint32_t icon);
    void ClearCache();
    void MakeDirty(const std::string& filepath);

protected:
    void _Draw_Impl() override;

private:
    void DrawPathNavigation();
    bool DrawDirectoryItem(const std::shared_ptr<Moon::Directory>& dir);
    void DrawFileItem(const std::shared_ptr<Moon::File>& file);
    void DrawItemContextMenu(const std::shared_ptr<Moon::Directory>& dir);
    void DrawItemContextMenu(const std::shared_ptr<Moon::File>& file);
    void DrawBackgroundContextMenu();

    void LoadIconAsync(const std::shared_ptr<Moon::File>& file);
    uint32_t GetFileIcon(const std::shared_ptr<Moon::File>& file);

    // Helpers
    std::string GetDragDropType(const std::shared_ptr<Moon::File>& file);
    bool ValidateFileName(const std::string& name, std::string& errorMsg);
    bool RenameItem(const std::filesystem::path& oldPath, const std::string& newName);
    bool DeleteItem(const std::filesystem::path& filepath);
private:
    struct RenameState {
        std::string buffer;
        bool isActive = false;
        bool isDirectory = false;
        std::filesystem::path targetPath;
        std::string errorMessage;
    } m_renameState;

    struct DeleteState {
        std::string itemName;
        bool showConfirmation = false;
        bool isDirectory = false;
        std::filesystem::path targetPath;
    } m_deleteState;

    // Icons
    uint32_t m_folderIcon = -1;
    uint32_t m_defaultFileIcon = -1;
    float m_iconSize = 64.0f;

    std::unordered_map<std::string, uint32_t> m_fileIcons;
    std::unordered_set<std::string> m_loadingQueue;
    uint32_t m_placeholderIcon = -1;
    
    float m_treeViewWidth = 100.0f;
    bool m_showTreeView = true;
    bool m_dirty = true;

    // Settings
    static constexpr int MAX_CONCURRENT_LOADS = 2;
    static constexpr int CACHE_TIMEOUT_MS = 300000;
};
