#pragma once

#include "Core/Base.h"
#include "FileSystem/FileSystem.h"

#include <filesystem>

namespace Faint {

	class FileSystemUI {
	public:
		static Ref<Directory> m_CurrentDirectory;
		bool m_HasClickedOnFile;
		std::string m_SearchKeyword;

		FileSystemUI() {
			m_HasClickedOnFile = false;
		}

		void Draw();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void DrawFileTree(Ref<Directory> directory);
		void DrawDirectory(Ref<Directory> directory, uint32_t drawId);
		void DrawFile(Ref<File> file, uint32_t drawId);
	private:
		Ref<Directory> m_EditorDir;
		Ref<File> m_EditorFile;

		std::string renameTempValue;
	};
}