#include "File.h"
#include <FileSystem/Directory.h>
#include <iostream>

namespace Moon {
	File::File(std::weak_ptr<Directory> parentDir, const std::string& absolutePath)
		: m_parent(parentDir), m_absolutePath(absolutePath) {
		DetectFileType();
		CachePathInfo();
	}

	std::string File::GetRelativePath() const {
		if (!m_cachedRelativePath.empty()) return m_cachedRelativePath;
		if (auto parent = m_parent.lock())
			m_cachedRelativePath = std::filesystem::relative(m_absolutePath, parent->GetRootPath()).string();
		else 
			m_cachedRelativePath = m_absolutePath.string();
		return m_cachedRelativePath;
	}

	std::string_view File::GetFileTypeAsString() const noexcept {
		switch (m_fileType) {
		case FileType::TEXTURE: return "Texture";
		case FileType::MODEL: return "Model";
		case FileType::FONT: return "Model";
		case FileType::MATERIAL: return "Model";
		case FileType::SCENE: return "Scene";
		case FileType::PREFAB: return "Project";
		case FileType::SCRIPT: return "Script";
		default: return "Unknown";
		}
	}

	bool File::Exists() const noexcept {
		return std::filesystem::exists(m_absolutePath);
	}
	
	bool File::IsModified() const {
		if (!Exists()) return false;
		auto current = std::filesystem::last_write_time(m_absolutePath);
		bool modified = !m_lastKnownWriteTime.has_value() || current > m_lastKnownWriteTime.value();
		if (modified) m_lastKnownWriteTime = current;
		return modified;
	}

	std::filesystem::file_time_type File::GetLastWriteTime() const {
		if (!m_lastKnownWriteTime.has_value()) {
			if (Exists()) {
				m_lastKnownWriteTime = std::filesystem::last_write_time(m_absolutePath);
			}
			else {
				m_lastKnownWriteTime = std::filesystem::file_time_type::min();
			}
		}
		return *m_lastKnownWriteTime;
	}

	void File::DetectFileType() {
		m_name = m_absolutePath.filename().string();
		m_extension = m_absolutePath.extension().string();
	}

	void File::CachePathInfo() {
		std::string ext = m_extension;
		//std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext == ".png" || ext == ".jpg" || ext == ".dds") m_fileType = FileType::TEXTURE;
		if (ext == ".mnmat" || ext == ".ftmat") m_fileType = FileType::MATERIAL;
		if (ext == ".scene" || ext == ".mnscene") m_fileType = FileType::SCENE;
		if (ext == ".project") m_fileType = FileType::PROJECT;
		if (ext == ".prefab") m_fileType = FileType::PREFAB;
		if (ext == ".lua") m_fileType = FileType::SCRIPT;
		if (ext == ".model") m_fileType = FileType::MESH;
		if (ext == ".obj" || ext == ".fbx" || ext == ".blend") m_fileType = FileType::MODEL;
		if (ext == ".wav" || ext == ".mp3") m_fileType = FileType::AUDIO;
		if (ext == ".ftshader" || ext == ".shader" ||
			ext == ".fx" || ext == ".vert" || ext == ".frag") m_fileType = FileType::FX;
	}
}
