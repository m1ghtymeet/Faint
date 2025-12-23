#include "FontManager.h"
#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>
#include <Util/MSDFGenerator.h>
#include <unordered_map>

namespace Moon::FontManager {
    std::unordered_map<std::string, std::shared_ptr<Font>> m_loadedFonts;
}

std::shared_ptr<Font> Moon::FontManager::LoadFont(const std::string& fontPath, float size) {
    std::string key = fontPath + "_" + std::to_string((int)size);
    if (m_loadedFonts.find(key) != m_loadedFonts.end())
        return m_loadedFonts[key];

    std::filesystem::path path(fontPath);
    if (!path.is_absolute())
        path = FileSystem::Root + path.string();
    
    const std::string parent = Moon::FileSystem::GetParentPath(path.string());
    const std::string stem = path.stem().string();
    std::string atlasPng = parent + stem + "_msdf.atlas.png";
    std::string atlasJson = parent + stem + "_msdf.atlas.json";
    if (Moon::FileSystem::FileExists(atlasPng, true) &&
        Moon::FileSystem::FileExists(atlasJson, true))
    {
        auto font = std::make_shared<Font>(atlasPng, atlasJson, size);
        m_loadedFonts[key] = font;
        HZ_CORE_INFO("Font loaded successfully (existing): {} @ {}px", fontPath, size);
        return font;
    }

    std::string generatedPng, generatedJson;
    if (GenerateMSDF(path.string(), size, generatedPng, generatedJson)) {
        auto font = std::make_shared<Font>(generatedPng, generatedJson, size);
        m_loadedFonts[key] = font;
        HZ_CORE_INFO("Font loaded successfully: {} @ {}px", fontPath, size);
        return font;
    }

    HZ_CORE_ERROR("Failed to load font: {}", fontPath);
    return nullptr;
}

bool Moon::FontManager::GenerateMSDF(const std::string& ttfPath, float size, std::string& outPng, std::string& outJson) {
    HZ_CORE_INFO("Generating MSDF for: {} @ {}px", ttfPath, size);

    if (!std::filesystem::exists(ttfPath)) {
        HZ_CORE_ERROR("TTF file not found: {}", ttfPath);
        return false;
    }

    Tools::MSDFGenerator::Atlas atlas;
    std::string jsonStr;
    std::string charset =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        "0123456789!@#$%^&*()_+-=[]{}|;':\",./<>? ";

    if (!Tools::MSDFGenerator::GenerateFromTTF(ttfPath, size, charset, atlas, jsonStr)) {
        HZ_CORE_ERROR("GenerateFromTTF failed!");
        return false;
    }

    std::filesystem::path ttf(ttfPath);
    std::string stem = ttf.stem().string();
    std::string dir = ttf.parent_path().string();
    outPng = dir + "/" + stem + "_msdf.atlas.png";
    outJson = dir + "/" + stem + "_msdf.atlas.json";

    HZ_CORE_INFO("Saving PNG: {}", outPng);
    HZ_CORE_INFO("Saving JSON: {}", outJson);

    Tools::MSDFGenerator::SaveAtlasAsPNG(atlas.pixels, atlas.width, atlas.height, outPng);
    std::ofstream(outJson) << jsonStr;

    HZ_CORE_INFO("MSDF Generated: {} @ {}pt → {} glyphs", stem, size, atlas.glyphs.size());
    return true;
}

void Moon::FontManager::ScanFonts()
{
}
