#pragma once
#include "Core/Base.h"
#include <glm/glm.hpp>
#include "ini.h"

namespace Faint::Config {
	mINI::INIFile Begin(const std::string& path, bool read = false);
	void Write(mINI::INIFile, std::string& title, std::string& value);
	inline mINI::INIStructure currentINI;

	inline glm::ivec2 finalImageResolution = { 1280, 720 };
}