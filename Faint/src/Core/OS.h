#pragma once

#include <string>

namespace Moon {
	class OS {
	public:
		OS() = delete;

		/** Copies text to the system clipboard. */
		static void CopyToClipboard(const std::string& value);

		/** Retrieves text from the system clipboard. Returns empty string if clipboard is empty or contains non-text. */
		static std::string GetFromClipboard();
		
		/** Opens a file or URL with its default associated application (like double-clicking). */
		static bool OpenIn(const std::string& filePath);

		/** Revea;s the file or folder in the native file explorer (selects it). */
		static bool ShowInFileExplorer(const std::string& filePath);

		/** Returns the path to the current executable (useful for relative paths, logging, etc.) */
		static std::string GetExecutablePath();

		/** Returns the directory containing the current executable. */
		static std::string GetExecutableDirectory();
	};
}