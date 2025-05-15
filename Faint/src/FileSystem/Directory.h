#pragma once

#include "Core/Base.h"

namespace Faint {
	
	class File;

	class Directory {
	public:
		std::string Name;
		std::string FullPath;
		Ref<Directory> Parent;
		std::vector<Ref<Directory>> Directories;
		std::vector<Ref<File>> Files;

		Directory() = default;
		Directory(const std::string& path) {
			Files = std::vector<Ref<File>>();
			Directories = std::vector<Ref<Directory>>();
			Name = FileSystem::AbsoluteToRelative(path);
			FullPath = path;
		}
		~Directory() = default;

		std::string GetName() const { return Name; }
		//std::string GetFullPath() const { return }
	};
}