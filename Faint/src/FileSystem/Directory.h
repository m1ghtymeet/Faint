#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>

namespace Moon {
	class File;

	class Directory {
	public:
		using Ptr = std::shared_ptr<Directory>;
		using WeakPtr = std::weak_ptr<Directory>;

		Directory(const std::filesystem::path& path, WeakPtr parent = {});

		// Non-copyable
		Directory(const Directory&) = delete;
		Directory& operator=(const Directory&) = delete;

		// Accessors
		std::string GetName() const noexcept { return m_name; }
		const std::filesystem::path& GetFullPath() const noexcept { return m_fullPath; }
		const std::filesystem::path& GetRootPath() const noexcept { return m_rootPath; }
		WeakPtr GetParent() const noexcept { return m_parent; }

		const std::vector<Ptr>& GetDirectories() const noexcept { return m_directories; }
		const std::vector<std::shared_ptr<File>>& GetFiles() const noexcept { return m_files; }

		// Navigation
		Ptr FindDirectory(const std::string& name) const;
		std::shared_ptr<File> FindFile(const std::string& name) const;

		// Path utilities
		std::vector<Directory::Ptr> GetPathFromRoot() const;
		std::string GetPathFromRoot(const std::string& path) const;
		std::string GetRelativePath(const std::filesystem::path& path) const;

		// Scanning
		void Scan(bool recursive = true);
		void Clear();

		// Modification
		bool HasChanged() const;
		void RefreshIfNeeded();

		std::weak_ptr<Directory> m_self;

	private:
		void ScanDirectory(const std::filesystem::path& path);

	private:
		std::filesystem::path m_fullPath;
		std::filesystem::path m_rootPath;
		std::string m_name;

		WeakPtr m_parent;
		std::vector<Ptr> m_directories;
		std::vector<std::shared_ptr<File>> m_files;

		mutable std::filesystem::file_time_type m_lastScanTime;
		mutable bool m_needsRescan = true;
	};
}