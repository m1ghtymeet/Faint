#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace msdfgen {
	typedef unsigned unicode_t;
}

struct Glyph {
	msdfgen::unicode_t codepoint = 0;
	float advance = 0.0f;
	glm::vec2 size = glm::vec2(0, 0);      // Pixel size
	glm::vec2 bearing = glm::vec2(0, 0);   // Bearing
	glm::vec2 atlasPos = glm::vec2(0, 0);  // UV position in atlas
	glm::vec2 atlasSize = glm::vec2(0, 0); // UV size
};

class Font {
public:
	Font(const std::string& path, unsigned int pixelSize = 12.0f);
	Font(const std::string& atlasImagePath, const std::string& atlasJsonPath, float pixelSize = 12.0f);
	~Font();

	const Glyph& GetGlyph(msdfgen::unicode_t codepoint) const;
	const Glyph& GetGlyph(char c) const;
	unsigned int GetAtlasTextureID() const { return m_atlasTexture; }
	float GetPixelSize() const { return m_pixelSize; }

private:
	void LoadAtlas(const std::string& jsonPath);

	std::unordered_map<msdfgen::unicode_t, Glyph> m_glyphs;
	unsigned int m_atlasTexture = 0;
	float m_pixelSize = 12.0f;
};