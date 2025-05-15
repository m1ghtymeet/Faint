#include "Config.h"



namespace Faint {

	
	std::string Config::currentPath;
	mINI::INIStructure Config::currentINI;
	//mINI::INIFile Config::currentFile;

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
