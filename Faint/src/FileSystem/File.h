#pragma once

#include "Core/Base.h"

namespace Faint {
	
	enum class FileType
	{
		Unknown,
		Image,
		Material,
		Mesh,
		MeshAsset,
		Script,
		NetScript,
		Project,
		Prefab,
		Scene,
		Wad,
		Map,
		Assembly,
		Solution,
		Audio,
		UI,
		CSS,
		Sky,
		Env
	};

	class Directory;

	class File {
	public:
		File(Ref<Directory> parentDir, const std::string& absolutePath, const std::string& name, const std::string& type);
		~File() = default;

		std::string GetName() const { return name; }
		std::string GetExtension() const { return type; }
		std::string GetRelativePath() const { return relativePath; }
		std::string GetAbsolutePath() const { return absolutePath; }
		Ref<Directory> GetParent() const { return parent; }

		bool GetHasBeenModified() const { return modified; }
		void SetHasBeenModified(bool value) { modified = value; }

		FileType GetFileType() const;
		std::string GetFileTypeAsString() const;

		std::string Read() const;
		bool Exist() const;

	private:
		std::string type;
		std::string name;
		std::string relativePath;
		std::string absolutePath;
		bool modified;

		Ref<Directory> parent;
	};
}