#include "MSDFGenerator.h"
#include <Debug/Log.h>
#include <msdfgen.h>
#include <msdfgen-ext.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h> 
#include <nlohmann/json.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>

using namespace msdfgen;
using namespace nlohmann;

bool Moon::Tools::MSDFGenerator::GenerateFromTTF(const std::string& ttfPath, float fontSize, const std::string& charset, Atlas& outAtlas, std::string& outJson) {
    // Initialize FreeType
    FT_Library ftLib;
    if (FT_Init_FreeType(&ftLib)) {
        HZ_CORE_ERROR("Failed to initialize FreeType");
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ftLib, ttfPath.c_str(), 0, &face)) {
        HZ_CORE_ERROR("Failed to load font face: {}", ttfPath);
        FT_Done_FreeType(ftLib);
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(fontSize));

    // Initialize msdfgen
    FreetypeHandle* ft = initializeFreetype();
    if (!ft) {
        FT_Done_Face(face);
        FT_Done_FreeType(ftLib);
        return false;
    }

    FontHandle* font = loadFont(ft, ttfPath.c_str());
    if (!font) {
        deinitializeFreetype(ft);
        FT_Done_Face(face);
        FT_Done_FreeType(ftLib);
        return false;
    }

    const double pxRange = 4.0;
    const int padding = 2;
    std::vector<Glyph> glyphs;
    std::vector<std::vector<unsigned char>> glyphPixels;

    // Generate MSDF for each character
    for (char c : charset) {
        unicode_t codepoint = static_cast<unicode_t>(static_cast<unsigned char>(c));

        // Load glyph metrics from FreeType
        if (FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT)) {
            continue;
        }

        // Get metrics from FreeType (in pixels, 1/64th pixel precision)
        float advance = static_cast<float>(face->glyph->advance.x >> 6);
        float bearingX = static_cast<float>(face->glyph->metrics.horiBearingX >> 6);
        float bearingY = static_cast<float>(face->glyph->metrics.horiBearingY >> 6);
        float glyphWidth = static_cast<float>(face->glyph->metrics.width >> 6);
        float glyphHeight = static_cast<float>(face->glyph->metrics.height >> 6);

        // Load shape from msdfgen
        GlyphIndex glyphIndex;
        if (!getGlyphIndex(glyphIndex, font, codepoint)) {
            // Empty glyph (like space)
            Glyph g;
            g.codepoint = codepoint;
            g.advance = advance;
            g.size = glm::vec2(0, 0);
            g.bearing = glm::vec2(bearingX, bearingY);
            g.atlasPos = glm::vec2(0, 0);  // Will be set during packing
            g.atlasSize = glm::vec2(0, 0);
            glyphs.push_back(g);
            glyphPixels.push_back({});
            continue;
        }

        Shape shape;
        if (!loadGlyph(shape, font, glyphIndex)) {
            continue;
        }

        shape.normalize();
        edgeColoringSimple(shape, 3.0);

        // Calculate bounds for MSDF generation
        Shape::Bounds bounds = shape.getBounds(pxRange);
        double l = bounds.l, b = bounds.b, r = bounds.r, t = bounds.t;

        // MSDF atlas dimensions (with padding)
        int atlasW = static_cast<int>(std::ceil(r - l)) + padding * 2;
        int atlasH = static_cast<int>(std::ceil(t - b)) + padding * 2;

        if (atlasW <= padding * 2 || atlasH <= padding * 2) {
            // Empty shape
            Glyph g;
            g.codepoint = codepoint;
            g.advance = advance;
            g.size = glm::vec2(glyphWidth, glyphHeight);
            g.bearing = glm::vec2(bearingX, bearingY);
            g.atlasPos = glm::vec2(0, 0);
            g.atlasSize = glm::vec2(0, 0);
            glyphs.push_back(g);
            glyphPixels.push_back({});
            continue;
        }

        // Generate MSDF
        Bitmap<float, 3> sdf(atlasW, atlasH);
        generateMSDF(sdf, shape, pxRange,
            Vec2(1.0, 1.0),
            Vec2(-l + padding, -b + padding));

        // Convert to RGBA
        std::vector<unsigned char> pixels(atlasW * atlasH * 4);
        for (int y = 0; y < atlasH; ++y) {
            for (int x = 0; x < atlasW; ++x) {
                int idx = (y * atlasW + x) * 4;
                float r = sdf(x, y)[0];
                float g = sdf(x, y)[1];
                float b = sdf(x, y)[2];
                pixels[idx + 0] = static_cast<unsigned char>(std::clamp(r * 255.f, 0.0f, 255.0f));
                pixels[idx + 1] = static_cast<unsigned char>(std::clamp(g * 255.f, 0.0f, 255.0f));
                pixels[idx + 2] = static_cast<unsigned char>(std::clamp(b * 255.f, 0.0f, 255.0f));
                pixels[idx + 3] = 255;
            }
        }

        // Store glyph metadata
        // مهم: size و bearing از FreeType می‌آیند (برای positioning)
        // atlasPos و atlasSize در packing set می‌شوند
        Glyph g;
        g.codepoint = codepoint;
        g.advance = advance;
        g.size = glm::vec2(glyphWidth, glyphHeight);
        g.bearing = glm::vec2(bearingX, bearingY);

        // Temporary: store MSDF atlas dimensions (با padding)
        // این فقط برای packing است و بعداً override می‌شود
        g.atlasPos = glm::vec2(0, 0);
        g.atlasSize = glm::vec2(static_cast<float>(atlasW), static_cast<float>(atlasH));

        glyphs.push_back(g);
        glyphPixels.push_back(std::move(pixels));
    }

    // Cleanup
    destroyFont(font);
    deinitializeFreetype(ft);
    FT_Done_Face(face);
    FT_Done_FreeType(ftLib);

    if (glyphs.empty()) {
        HZ_CORE_ERROR("No glyphs generated!");
        return false;
    }

    // ========================================
    // Atlas Packing
    // ========================================
    const int atlasSize = 1024;
    outAtlas.width = atlasSize;
    outAtlas.height = atlasSize;
    outAtlas.pixels.assign(atlasSize * atlasSize * 4, 0);

    int x = 0, y = 0, rowHeight = 0;

    for (size_t i = 0; i < glyphs.size(); ++i) {
        const auto& srcPixels = glyphPixels[i];

        if (srcPixels.empty()) {
            // Empty glyph (space, etc.)
            glyphs[i].atlasPos = glm::vec2(0, 0);
            glyphs[i].atlasSize = glm::vec2(0, 0);
            continue;
        }

        // Get MSDF atlas dimensions (already includes padding)
        int w = static_cast<int>(glyphs[i].atlasSize.x);
        int h = static_cast<int>(glyphs[i].atlasSize.y);

        // Check if we need a new row
        if (x + w > atlasSize) {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        if (y + h > atlasSize) {
            HZ_CORE_ERROR("Atlas overflow! Need larger atlas size.");
            return false;
        }

        rowHeight = std::max(rowHeight, h);

        // Copy pixels to atlas
        for (int py = 0; py < h; ++py) {
            for (int px = 0; px < w; ++px) {
                int srcIdx = (py * w + px) * 4;
                int dstIdx = ((y + py) * atlasSize + (x + px)) * 4;
                std::copy_n(srcPixels.begin() + srcIdx, 4, outAtlas.pixels.begin() + dstIdx);
            }
        }

        // Set normalized UV coordinates (0-1)
        glyphs[i].atlasPos = glm::vec2(
            static_cast<float>(x + padding) / static_cast<float>(atlasSize),
            static_cast<float>(y + padding) / static_cast<float>(atlasSize)
        );
        glyphs[i].atlasSize = glm::vec2(
            static_cast<float>(w - padding * 2) / static_cast<float>(atlasSize),
            static_cast<float>(h - padding * 2) / static_cast<float>(atlasSize)
        );

        x += w;
    }

    outAtlas.glyphs = std::move(glyphs);

    // ========================================
    // Generate JSON
    // ========================================
    json j;
    j["name"] = std::filesystem::path(ttfPath).stem().string();
    j["type"] = "msdf";
    j["size"] = fontSize;
    j["atlas"]["width"] = atlasSize;
    j["atlas"]["height"] = atlasSize;
    j["pxRange"] = pxRange;

    json glyphsJson = json::array();
    for (const auto& g : outAtlas.glyphs) {
        json gj;
        gj["unicode"] = g.codepoint;
        gj["advance"] = g.advance;

        // Plane bounds (pixel space) - برای positioning در runtime
        json planeBounds;
        planeBounds["left"] = g.bearing.x;
        planeBounds["bottom"] = g.bearing.y - g.size.y;
        planeBounds["right"] = g.bearing.x + g.size.x;
        planeBounds["top"] = g.bearing.y;
        gj["planeBounds"] = planeBounds;

        // Atlas bounds (normalized 0-1) - برای UV mapping
        json atlasBounds;
        atlasBounds["left"] = g.atlasPos.x;
        atlasBounds["bottom"] = g.atlasPos.y;
        atlasBounds["right"] = g.atlasPos.x + g.atlasSize.x;
        atlasBounds["top"] = g.atlasPos.y + g.atlasSize.y;
        gj["atlasBounds"] = atlasBounds;

        glyphsJson.push_back(gj);
    }
    j["glyphs"] = glyphsJson;

    outJson = j.dump(4);

    HZ_CORE_INFO("Generated {} glyphs, atlas usage: {:.1f}%",
        outAtlas.glyphs.size(),
        (static_cast<float>(y + rowHeight) / atlasSize) * 100.0f);

    return true;
}

void Moon::Tools::MSDFGenerator::SaveAtlasAsPNG(const std::vector<unsigned char>& pixels, int w, int h, const std::string& path) {
    stbi_flip_vertically_on_write(false);
    stbi_write_png(path.c_str(), w, h, 4, pixels.data(), w * 4);
    HZ_CORE_INFO("Saved PNG atlas: {}", path);
}

void Moon::Tools::MSDFGenerator::SaveAtlasAsJSON(const Atlas& atlas, const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        json j;
        j["name"] = "generated";
        j["size"] = 48.0f; // یا از پارامتر
        j["atlas"]["width"] = atlas.width;
        j["atlas"]["height"] = atlas.height;
        j["pxRange"] = 4.0;

        json glyphsJson = json::array();
        for (const auto& g : atlas.glyphs) {
            json gj;
            gj["char"] = std::string(1, g.codepoint);
            gj["advance"] = g.advance;
            gj["width"] = g.size.x;
            gj["height"] = g.size.y;
            gj["bearingX"] = g.bearing.x;
            gj["bearingY"] = g.bearing.y;
            gj["atlas"]["x"] = g.atlasPos.x;
            gj["atlas"]["y"] = g.atlasPos.y;
            gj["atlas"]["w"] = g.atlasSize.x;
            gj["atlas"]["h"] = g.atlasSize.y;
            glyphsJson.push_back(gj);
        }
        j["glyphs"] = glyphsJson;

        file << j.dump(4);
        HZ_CORE_INFO("Saved JSON atlas: {}", path);
    }
}