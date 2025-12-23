#pragma once
#include "Core/Base.h"
#include <glm/glm.hpp>
#include "../vendor/ini.h"

namespace Moon::Config {
	mINI::INIFile Begin(const std::string& path, bool read = false);
	void Write(mINI::INIFile, std::string& title, std::string& value);
	inline mINI::INIStructure currentINI;

	inline glm::ivec2 finalImageResolution = { 1280, 720 };

	//using Value = std::variant<int, std::string, std::string>; // Third string can represent complex values like {}
	//
	//class ConfigReader {
	//public:
	//	bool Load(const std::string& filepath);
	//	void PrintValues() const;
	//
	//private:
	//	std::unordered_map<std::string, Value> values;
	//};
}