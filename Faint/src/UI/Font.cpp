#include "Font.h"
#include <AssetManagment/Serializable.h>
#include <Debug/Log.h>
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stb_image.h>
#include <utf8.h>

#include <fstream>

Font::Font(const std::string& path, unsigned int pixelSize) {
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		HZ_CORE_ERROR("Failed to init FreeType Library");

	FT_Face face;
	if (FT_New_Face(ft, path.c_str(), 0, &face))
		HZ_CORE_ERROR("Failed to load font: " + path);

	FT_Set_Pixel_Sizes(face, 0, pixelSize);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			continue;

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Font::Font(const std::string& atlasImagePath, const std::string& atlasJsonPath, float pixelSize) {
    m_pixelSize = pixelSize;

    if (!std::filesystem::exists(atlasImagePath)) {
        HZ_CORE_ERROR("Atlas PNG not found: {}", atlasImagePath);
        return;
    }

    int w, h, channels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(atlasImagePath.c_str(), &w, &h, &channels, 4);
    if (!data) {
        HZ_CORE_ERROR("stbi_load failed: {}", stbi_failure_reason());
        HZ_CORE_ERROR("Path: {}", atlasImagePath);
        return;
    }

    glGenTextures(1, &m_atlasTexture);
    glBindTexture(GL_TEXTURE_2D, m_atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    HZ_CORE_INFO("Atlas loaded: {}x{} ({} channels)", w, h, channels);

    LoadAtlas(atlasJsonPath);
}

Font::~Font() {
    if (m_atlasTexture) glDeleteTextures(1, &m_atlasTexture);
}

const Glyph& Font::GetGlyph(msdfgen::unicode_t codepoint) const {
    static Glyph fallback = { 0, 0, {0,0}, {0,0}, {0,0}, {0,0} };
    auto it = m_glyphs.find(codepoint);
    return it != m_glyphs.end() ? it->second : fallback;
}

const Glyph& Font::GetGlyph(char c) const {
    msdfgen::unicode_t codepoint = static_cast<msdfgen::unicode_t>(static_cast<unsigned char>(c));
    return GetGlyph(codepoint);
}

void Font::LoadAtlas(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        HZ_CORE_ERROR("Failed to open JSON: {}", jsonPath);
        return;
    }

    json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        HZ_CORE_ERROR("JSON parse error: {}", e.what());
        return;
    }

    if (!j.contains("glyphs")) {
        HZ_CORE_ERROR("JSON missing 'glyphs' array");
        return;
    }

    int atlasWidth = j.value("atlas", json::object()).value("width", 1024);
    int atlasHeight = j.value("atlas", json::object()).value("height", 1024);

    for (auto& item : j["glyphs"].items()) {
        auto& gj = item.value();
        msdfgen::unicode_t codepoint = 0;

        // دریافت codepoint
        if (gj.contains("unicode")) {
            codepoint = gj["unicode"].get<msdfgen::unicode_t>();
        }
        else if (gj.contains("char")) {
            std::string s = gj["char"].get<std::string>();
            if (!s.empty()) {
                codepoint = static_cast<msdfgen::unicode_t>(static_cast<unsigned char>(s[0]));
            }
        }

        if (codepoint == 0) continue;

        Glyph g;
        g.codepoint = codepoint;

        // Atlas Bounds (normalized 0-1)
        if (gj.contains("atlasBounds")) {
            auto& ab = gj["atlasBounds"];
            float l = ab.value("left", 0.0f);
            float b = ab.value("bottom", 0.0f);
            float r = ab.value("right", 0.0f);
            float t = ab.value("top", 0.0f);

            g.atlasPos = glm::vec2(l, b);
            g.atlasSize = glm::vec2(r - l, t - b);
        }

        // Plane Bounds (pixel space)
        if (gj.contains("planeBounds")) {
            auto& pb = gj["planeBounds"];
            float left = pb.value("left", 0.0f);
            float bottom = pb.value("bottom", 0.0f);
            float right = pb.value("right", 0.0f);
            float top = pb.value("top", 0.0f);

            g.size.x = right - left;
            g.size.y = top - bottom;
            g.bearing.x = left;
            g.bearing.y = top;
        }

        g.advance = gj.value("advance", 0.0f);

        m_glyphs[codepoint] = g;

        // Debug log
        HZ_CORE_INFO("Loaded glyph '{}' (U+{:04X}): adv={:.1f}, size=({:.1f},{:.1f})",
            static_cast<char>(codepoint), codepoint, g.advance, g.size.x, g.size.y);
    }

    HZ_CORE_INFO("Loaded {} glyphs from MSDF atlas", m_glyphs.size());
}
