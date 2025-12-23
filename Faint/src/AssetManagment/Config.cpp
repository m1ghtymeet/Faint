#include "Config.h"

namespace Moon::Config {

	std::string currentPath;

	mINI::INIFile Config::Begin(const std::string& path, bool read)
	{
		mINI::INIFile file(path);
		currentPath = path;
		
		mINI::INIStructure ini;

		if (read)
			file.read(ini);
		currentINI = ini;

		return file;
	}

	void Config::Write(mINI::INIFile file, std::string& title, std::string& value) {

		
	}
}
