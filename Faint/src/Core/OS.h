#pragma once

#include <string>
#include "FileSystem/FileSystem.h"

namespace Faint {

	class OS {
	public:
		static void CopyToClipboard(const std::string& value);
		static std::string GetFromClipboard();
		static void OpenIn(const std::string& filePath);
		static int RenameDirectory(const Ref<Directory>& dir, const std::string& newName);
		static void ShowInFileExplorer(const std::string& filePath);
		static std::string CompileSln(const std::string& slnPath);

		static int Subprocess(const std::string& command, std::string& out, std::string& err);
	};
}