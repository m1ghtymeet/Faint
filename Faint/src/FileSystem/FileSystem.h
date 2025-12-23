#pragma once
#include <FileSystem/Directory.h>
#include <FileSystem/MaultFile.h>
#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace Moon {

	class File;

	class FileSystem {
	public:
		// --- File Operations ---
		static bool FileExists(const std::string& path, bool absolute = false);
		static std::string ReadFile(const std::string& path, bool absolute = false);
		static bool BeginWriteFile(const std::string path, bool absolute = false);
		static bool WriteLine(const std::string line);
		static void EndWriteFile();

		// --- Directory Operations ---
		static void RegisterDirectory(const std::filesystem::path& p, Directory::WeakPtr d);
		static std::vector<Directory::Ptr> GetPathFromRoot(const Directory* dir);

		// --- Build Tools ---
		static void MountPak(const std::string& pakPath);
		static void UnmountPak();
		static bool FileExistsInPak(const std::string& path);
		static void ShowAllFilesInPak();

		// --- Paths ---
		static std::string Root;
		static std::string EnginePath;
		static std::shared_ptr<Directory> RootDirectory;
		static std::filesystem::path m_projectPath;

		static std::string AbsoluteToRelative(const std::string& path);
		static std::string RelativeToAbsolute(const std::string& path);
		static std::string GetParentPath(const std::string& fullPath);
		static std::shared_ptr<Directory> GetFileTree();
		static std::shared_ptr<File> GetFile(const std::string& path);
		static std::shared_ptr<Directory> GetDirectory(const std::string& path);
		static std::string GetFileNameFromPath(const std::string& path);
		static void SetRootDirectory(const std::string path);

		static bool CompressDirectoryToZip(const std::string& sourceDir, const std::string& zipPath);
		static bool CompressFileToZip(const std::string& srcFile, const std::string& zipPath);
		static bool MakeDirectory(const std::string& path, bool absolute = false);
		static bool DirectoryExists(const std::string& path, bool absolute = false);

		static uintmax_t DeleteFileFromPath(const std::string& path);
		static uintmax_t DeleteFolder(const std::string& path);
	
	private:
		static std::ofstream m_fileWriter;
		static std::unordered_map<std::filesystem::path, Moon::Directory::WeakPtr> m_pathToDir;
		static std::vector<std::shared_ptr<Tools::MaultFile>> m_paks;
		static std::mutex m_pakMutex;
	};
}