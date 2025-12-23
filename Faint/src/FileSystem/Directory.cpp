#include "Directory.h"
#include "File.h"
#include "FileSystem.h"
#include <Debug/Log.h>
#include <algorithm>

Moon::Directory::Directory(const std::filesystem::path& path, WeakPtr parent)
	: m_fullPath(std::filesystem::absolute(path)), m_parent(parent) {
	m_name = m_fullPath.filename().string();
    if (m_name.empty()) {
        if (m_fullPath.has_root_name() && !m_fullPath.has_relative_path()) {
            m_name = m_fullPath.root_name().string();
        }
        else {
            m_name = m_fullPath.parent_path().filename().string();
        }
    }
    if (auto locked = m_parent.lock())
        m_rootPath = locked->GetRootPath();
    else
        m_rootPath = m_fullPath;
}

Moon::Directory::Ptr Moon::Directory::FindDirectory(const std::string& name) const {
	return Ptr();
}

std::shared_ptr<Moon::File> Moon::Directory::FindFile(const std::string& name) const
{
	return std::shared_ptr<File>();
}

std::vector<Moon::Directory::Ptr> Moon::Directory::GetPathFromRoot() const {
    return Moon::FileSystem::GetPathFromRoot(this);
}

std::string Moon::Directory::GetPathFromRoot(const std::string& path) const {
    return std::filesystem::relative(path, m_rootPath).string();
}

void Moon::Directory::Scan(bool recursive) {
    if (m_self.expired()) {
        if (auto parent = m_parent.lock()) {
            for (auto& dir : parent->m_directories) {
                if (dir.get() == this) {
                    m_self = dir;
                    break;
                }
            }
        }
    }

    Clear();
    ScanDirectory(m_fullPath);
    m_lastScanTime = std::filesystem::last_write_time(m_fullPath);
    m_needsRescan = false;

    if (recursive) {
        for (auto& dir : m_directories) {
            dir->Scan(true);
        }
    }
}

void Moon::Directory::Clear() {
    m_directories.clear();
    m_files.clear();
}

bool Moon::Directory::HasChanged() const {
    if (!std::filesystem::exists(m_fullPath)) return true;
    auto current = std::filesystem::last_write_time(m_fullPath);
    return current > m_lastScanTime;
}

void Moon::Directory::RefreshIfNeeded() {
    if (HasChanged())
        Scan(true);
}

void Moon::Directory::ScanDirectory(const std::filesystem::path& path) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(
            path, std::filesystem::directory_options::skip_permission_denied)) {

            if (entry.is_directory()) {
                auto subDir = std::make_shared<Directory>(entry.path(), m_self);
                subDir->m_parent = m_self;
                subDir->m_self = subDir;

                m_directories.push_back(subDir);
                FileSystem::RegisterDirectory(entry.path(), subDir);
            }
            else if (entry.is_regular_file()) {
                auto file = std::make_shared<File>(m_self, entry.path().string());  // یا m_parent
                m_files.push_back(file);
            }
        }

        std::ranges::sort(m_directories, [](const Ptr& a, const Ptr& b) {
            return a->GetName() < b->GetName();
            });
        std::ranges::sort(m_files, [](const auto& a, const auto& b) {
            return a->GetName() < b->GetName();
            });
    }
    catch (const std::exception& e) {
        //LOG_ERROR("Scan error: {}", e.what());
    }
}
