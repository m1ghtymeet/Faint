#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <chrono>

namespace Moon {
	class Directory;
	enum class FileType {
		UNKNOWN,
		TEXTURE,
		MATERIAL,
		MESH,
		MODEL,
		FONT,
		SCRIPT,
		PROJECT,
		PREFAB,
		SCENE,
		AUDIO,
		FX
	};

	class File {
	public:
		File(std::weak_ptr<Directory> parentDir,
			const std::string& absolutePath);
		~File() = default;

		// Non-copyable, movable
		File(const File&) = delete;
		File& operator=(const File&) = delete;
		File(File&&) noexcept = default;
		File& operator=(File&&) noexcept = default;

		// Getters
		std::string GetName() const noexcept { return m_name; };
		std::string GetExtension() const noexcept { return m_extension; };
		const std::filesystem::path& GetAbsolutePath() const noexcept { return m_absolutePath; }
		const std::filesystem::path& GetFullPath() const noexcept { return m_absolutePath; }
		std::string GetRelativePath() const;
		std::weak_ptr<Directory> GetParent() const noexcept { return m_parent; }

		// File info
		FileType GetFileType() const noexcept { return m_fileType; };
		std::string_view GetFileTypeAsString() const noexcept;

		// File system operations
		bool Exists() const noexcept;
		std::string ReadAllText() const;
		std::vector<uint8_t> ReadAllBytes() const;

		// Modification tracking
		bool IsModified() const;
		void MarkModified() noexcept { m_lastKnownWriteTime = {}; }
		std::filesystem::file_time_type GetLastWriteTime() const;

	private:
		void DetectFileType();
		void CachePathInfo();

	private:
		std::filesystem::path m_absolutePath;
		std::weak_ptr<Directory> m_parent;

		std::string m_name;
		std::string m_extension;

		mutable std::string m_cachedRelativePath;
		mutable std::optional<std::filesystem::file_time_type> m_lastKnownWriteTime;

		FileType m_fileType = FileType::UNKNOWN;
	};
}