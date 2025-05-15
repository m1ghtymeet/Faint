#pragma once

#include "Core/Base.h"

#include "ini.h"

namespace Faint {

	class Config {
	public:

		static mINI::INIFile Begin(const std::string& path, bool read = false);
		static void Write(mINI::INIFile, std::string& title, std::string& value);

		static mINI::INIStructure currentINI;

	private:
		static std::string currentPath;
		
		//static mINI::INIFile currentFile;
	};
}