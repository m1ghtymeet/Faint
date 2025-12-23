#pragma once
#include <UI/Font.h>

namespace Moon::Tools::MSDFGenerator {

	struct Atlas {
		int width = 0, height = 0;
		std::vector<unsigned char> pixels; // RGBA
		std::vector<Glyph> glyphs;
	};

	bool GenerateFromTTF(
		const std::string& ttfPath,
		float fontSize,
		const std::string& charset,
		Atlas& outAtlas,
		std::string& outJson
	);

	void SaveAtlasAsPNG(const std::vector<unsigned char>& pixels, int w, int h, const std::string& path);
	void SaveAtlasAsJSON(const Atlas& atlas, const std::string& path);
}