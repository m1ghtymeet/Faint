#include "FileSystem.h"
#include "File.h"
#include "Directory.h"
#include "Core/String.h"

#include <Debug/Log.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <sstream>
#include <zstd.h>

std::string Moon::FileSystem::Root = "";
std::string Moon::FileSystem::EnginePath = "";
std::filesystem::path Moon::FileSystem::m_projectPath = "";
std::shared_ptr<Moon::Directory> Moon::FileSystem::RootDirectory;
std::ofstream Moon::FileSystem::m_fileWriter;
std::unordered_map<std::filesystem::path, Moon::Directory::WeakPtr> Moon::FileSystem::m_pathToDir;
std::vector<std::shared_ptr<Moon::Tools::MaultFile>> Moon::FileSystem::m_paks;
std::mutex Moon::FileSystem::m_pakMutex;

bool Moon::FileSystem::FileExists(const std::string& path, bool absolute) {
    std::string fullPath = absolute ? path : Root + path;
    {
        std::lock_guard<std::mutex> lock(m_pakMutex);
        for (const auto& pak : m_paks) {
            if (pak && pak->FileExists(fullPath)) return true;
        }
    }
    return std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath);
}

std::string Moon::FileSystem::ReadFile(const std::string& path, bool absolute) {
    std::string finalPath = absolute ? path : Root + path;

    {
        std::lock_guard<std::mutex> lock(m_pakMutex);
        for (const auto& pak : m_paks) {
            if (pak && pak->FileExists(finalPath)) {
                auto data = pak->ReadFile(finalPath);
                if (!data.empty()) {
                    std::string text(data.begin(), data.end());
                    if (text.size() >= 3 && (uint8_t)text[0] == 0xEF && (uint8_t)text[1] == 0xBB && (uint8_t)text[2] == 0xBF) {
                        text.erase(0, 3);
                    }
                    return text;
                }
            }
        }
    }

    std::ifstream file(finalPath, std::ios::binary);
    if (!file) return "";

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    std::string text(buffer.begin(), buffer.end());

    // Remove BOM
    if (text.size() >= 3 &&
        (uint8_t)text[0] == 0xEF &&
        (uint8_t)text[1] == 0xBB &&
        (uint8_t)text[2] == 0xBF) {
        text.erase(0, 3);
    }
    return text;
}

bool Moon::FileSystem::BeginWriteFile(const std::string path, bool absolute) {
    m_fileWriter.close();
    m_fileWriter.open(absolute ? path : FileSystem::Root + path);
    return m_fileWriter.is_open();
}

bool Moon::FileSystem::WriteLine(const std::string line) {
    if (!m_fileWriter.is_open()) return false;
    m_fileWriter << line.c_str();
    return true;
}

void Moon::FileSystem::EndWriteFile() {
    if (m_fileWriter.is_open()) m_fileWriter.close();
}

namespace Moon {

    void FileSystem::RegisterDirectory(const std::filesystem::path& p, Directory::WeakPtr d) {
        m_pathToDir[std::filesystem::absolute(p)] = d;
    }

    std::vector<Directory::Ptr> FileSystem::GetPathFromRoot(const Directory* dir) {
        std::vector<Directory::Ptr> result;

        if (!dir) return result;

        std::filesystem::path cur = dir->GetFullPath();

        while (true) {
            auto it = m_pathToDir.find(cur);
            if (it == m_pathToDir.end() || it->second.expired())
                break;

            auto shared = it->second.lock();
            if (!shared)
                break;

            result.push_back(shared);

            // stop at the project root
            if (cur == shared->GetRootPath())
                break;

            if (cur == cur.root_path())
                break;

            cur = cur.parent_path();
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    void FileSystem::MountPak(const std::string& pakPath) {
        auto pak = std::make_shared<Tools::MaultFile>(pakPath);
        if (pak->Mount()) {
            std::lock_guard<std::mutex> lock(m_pakMutex);
            m_paks.push_back(std::move(pak));
            HZ_CORE_INFO("[FileSystem] Mounted pak: {}", pakPath);
        }
        else {
            HZ_CORE_ERROR("[FileSystem] Failed to mount pak: " + pakPath);
        }
    }

    void FileSystem::UnmountPak() {
        std::lock_guard<std::mutex> lock(m_pakMutex);
        m_paks.clear();
    }

    bool FileSystem::FileExistsInPak(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_pakMutex);
        for (const auto& pak : m_paks) {
            if (pak && pak->FileExists(path)) return true;
        }
        return false;
    }

    void FileSystem::ShowAllFilesInPak() {
        std::lock_guard<std::mutex> lock(m_pakMutex);
        for (const auto& pak : m_paks) {
            if (!pak) continue;
            std::cout << "=== Pak: " << pak << " ===\n";
            //for (const auto& [name, entry] : pak->GetEntries()) {
            //    std::cout << name << " (" << entry.unCompressedSize << " bytes)\n";
            //}
        }
    }

    uintmax_t FileSystem::DeleteFileFromPath(const std::string& path) {
        return std::remove(path.c_str());
    }

    uintmax_t FileSystem::DeleteFolder(const std::string& path) {
        return std::filesystem::remove_all(path.c_str());
    }

    void FileSystem::SetRootDirectory(const std::string path)
    {
        Root = path;
        FileSystem::EnginePath = String::ReplaceSlash(std::filesystem::current_path().string()) + "/";
    }

    bool FileSystem::CompressDirectoryToZip(const std::string& sourceDir, const std::string& zipPath) {
        if (!std::filesystem::exists(sourceDir)) return false;

        std::ofstream out(zipPath, std::ios::binary);
        if (!out) return false;

        std::vector<Tools::VaultEntry> entries;
        uint64_t dataOffset = sizeof(uint32_t); // بعد از fileCount

        // --- placeholder برای fileCount ---
        uint32_t fileCount = 0;
        std::streampos fileCountPos = out.tellp();
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

        // --- فشرده‌سازی فایل‌ها ---
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
            if (!entry.is_regular_file()) continue;

            std::string relPath = std::filesystem::relative(entry.path(), sourceDir).generic_string();

            std::ifstream in(entry.path(), std::ios::binary);
            if (!in) continue;

            std::vector<uint8_t> raw((std::istreambuf_iterator<char>(in)), {});
            in.close();

            size_t bound = ZSTD_compressBound(raw.size());
            std::vector<uint8_t> compressed(bound);
            size_t cSize = ZSTD_compress(compressed.data(), bound, raw.data(), raw.size(), 5);
            if (ZSTD_isError(cSize)) {
                HZ_CORE_WARN("ZSTD compress failed for {}: {}", relPath, ZSTD_getErrorName(cSize));
                continue;
            }
            compressed.resize(cSize);

            Tools::VaultEntry e;
            e.name = relPath;
            e.offset = dataOffset;
            e.compressedSize = cSize;
            e.unCompressedSize = raw.size();
            entries.push_back(e);

            out.write(reinterpret_cast<const char*>(compressed.data()), cSize);
            dataOffset += cSize;
        }

        // --- نوشتن فهرست ---
        fileCount = static_cast<uint32_t>(entries.size());
        out.seekp(fileCountPos);
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));
        out.seekp(dataOffset);

        for (const auto& e : entries) {
            uint16_t nameLen = static_cast<uint16_t>(e.name.size());
            if (nameLen != e.name.size()) {
                HZ_CORE_ERROR("Filename too long: {}", e.name);
                continue;
            }
            out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            out.write(e.name.data(), nameLen);
            out.write(reinterpret_cast<const char*>(&e.offset), sizeof(e.offset));
            out.write(reinterpret_cast<const char*>(&e.compressedSize), sizeof(e.compressedSize));
            out.write(reinterpret_cast<const char*>(&e.unCompressedSize), sizeof(e.unCompressedSize));
        }

        out.close();
        HZ_CORE_INFO("Compressed directory: {} -> {} ({} files)", sourceDir, zipPath, entries.size());
        return true;
    }

    bool FileSystem::CompressFileToZip(const std::string& srcFile, const std::string& zipPath) {
        if (!FileExists(srcFile, true)) return false;

        std::ofstream out(zipPath, std::ios::binary);
        if (!out) return false;

        std::vector<Tools::VaultEntry> entries;
        uint64_t dataOffset = 0;

        uint32_t fileCount = 0;
        std::streampos fileCountPos = out.tellp();
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));
        dataOffset += sizeof(fileCount);

        std::string relPath = std::filesystem::path(srcFile).filename().string();

        std::ifstream in(srcFile, std::ios::binary);
        std::vector<uint8_t> raw((std::istreambuf_iterator<char>(in)), {});
        in.close();

        size_t bound = ZSTD_compressBound(raw.size());
        std::vector<uint8_t> compressed(bound);
        size_t cSize = ZSTD_compress(compressed.data(), bound, raw.data(), raw.size(), 19);
        if (ZSTD_isError(cSize)) {
            HZ_CORE_ERROR("ZSTD compress error: {}", ZSTD_getErrorName(cSize));
            return false;
        }
        compressed.resize(cSize);

        Tools::VaultEntry e;
        e.name = relPath;
        e.offset = dataOffset;
        e.compressedSize = cSize;
        e.unCompressedSize = raw.size();
        entries.push_back(e);

        out.write(reinterpret_cast<const char*>(compressed.data()), cSize);
        dataOffset += cSize;

        // --- فهرست ---
        fileCount = 1;
        out.seekp(fileCountPos);
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));
        out.seekp(dataOffset);

        for (const auto& e : entries) {
            uint16_t nameLen = static_cast<uint16_t>(e.name.size());
            out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            out.write(e.name.data(), nameLen);
            out.write(reinterpret_cast<const char*>(&e.offset), sizeof(e.offset));
            out.write(reinterpret_cast<const char*>(&e.compressedSize), sizeof(e.compressedSize));
            out.write(reinterpret_cast<const char*>(&e.unCompressedSize), sizeof(e.unCompressedSize));
        }

        out.close();
        return true;
    }

    std::string FileSystem::AbsoluteToRelative(const std::string& path)
    {
        const std::filesystem::path rootPath(Root);
        const std::filesystem::path absolutePath(path);
        return std::filesystem::relative(absolutePath, rootPath).generic_string();
    }

    std::string FileSystem::RelativeToAbsolute(const std::string& path)
    {
        return Root + path;
    }

    std::string FileSystem::GetParentPath(const std::string& fullPath) {

        std::filesystem::path pathObj(fullPath);
        auto returnvalue = pathObj.parent_path().string();
        return returnvalue + "/";
    }

    std::shared_ptr<Directory> FileSystem::GetFileTree() {
        if (!RootDirectory) {
            m_projectPath = Root;
            RootDirectory = std::make_shared<Directory>(m_projectPath);
            RootDirectory->m_self = RootDirectory;                     // root also gets a weak self
            RegisterDirectory(m_projectPath, RootDirectory);
            RootDirectory->Scan(true);
        }
        return RootDirectory;
    }

    std::shared_ptr<File> FileSystem::GetFile(const std::string& inPath)
    {
        std::string path = inPath;
        if (String::BeginsWith(path, "/") || String::BeginsWith(path, "\\"))
        {
            path = inPath.substr(1, inPath.size() - 1);
        }

        // Note, Might be broken on other platforms.
        auto splits = String::Split(path, '/');

        int currentDepth = -1;
        std::string currentDirName = ".";
        std::shared_ptr<Directory> currentDirComparator = RootDirectory;
        while (currentDirName == currentDirComparator->GetName())
        {
            currentDepth++;
            currentDirName = splits[currentDepth];

            // Find next directory
            for (auto& d : currentDirComparator->GetDirectories())
            {
                if (d->GetName() == currentDirName)
                {
                    currentDirComparator = d;
                }
            }

            // Find in files if can't find in directories.
            for (auto& f : currentDirComparator->GetFiles())
            {
                if (f->GetName() == currentDirName)
                {
                    return f;
                }
            }
        }

        return nullptr;
    }

    std::shared_ptr<Directory> FileSystem::GetDirectory(const std::string& path)
    {
        // Note, Might be broken on other platforms.
        auto splits = String::Split(path, '/');

        int currentDepth = -1;
        std::string currentDirName = ".";
        std::shared_ptr<Directory> currentDirComparator = RootDirectory;
        while (currentDirName == currentDirComparator->GetName())
        {
            currentDepth++;

            if (currentDepth >= splits.size())
            {
                return currentDirComparator;
            }

            currentDirName = splits[currentDepth];

            // Find next directory
            for (auto& d : currentDirComparator->GetDirectories())
            {
                if (d->GetName() == currentDirName)
                {
                    currentDirComparator = d;
                    continue;
                }
            }
        }

        return currentDirComparator;
    }

    std::string FileSystem::GetFileNameFromPath(const std::string& path)
    {
        const auto& split = String::Split(path, '/');
        return String::Split(split[split.size() - 1], '.')[0];
    }

    bool FileSystem::MakeDirectory(const std::string& path, bool absolute)
    {
        return std::filesystem::create_directories(absolute ? path : FileSystem::Root + path);
    }

    bool FileSystem::DirectoryExists(const std::string& path, bool absolute)
    {
        const std::string& finalPath = absolute ? path : Root + path;
        return std::filesystem::exists(finalPath) && std::filesystem::is_directory(finalPath);
    }
}