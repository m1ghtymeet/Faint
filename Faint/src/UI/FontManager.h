#pragma once
#include <string>
#include <memory>
#include "Font.h"

namespace Moon::FontManager {
	std::shared_ptr<Font> LoadFont(const std::string& fontPath, float size = 48.0f);
	bool GenerateMSDF(const std::string& ttfPath, float size, std::string& outPng, std::string& outJson);
	void ScanFonts();
}