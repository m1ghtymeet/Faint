#include "TextureLoader.h"
#include <Debug/Log.h>
#include <Types/Renderer/Texture.h>
#include <FileSystem/FileSystem.h>
#include <Core/String.h>
#include <Util/ImageTools.h>
#include <iostream>
#include <future>
#include <thread>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

Moon::Assets::Texture* Moon::Loaders::TextureLoader::Create(const std::string& filepath, const Options& opt) {
    if (filepath.empty() || !FileSystem::FileExists(filepath, true)) {
        HZ_CORE_ERROR("[TextureLoader] File not found: {}", filepath);
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(s_mutex);
	auto it = s_textures.find(filepath);
	if (it != s_textures.end()) {
		return it->second;
	}
	Moon::Rendering::Data::TextureData textureData;
    if (String::EndsWith(filepath, ".dds"))
        textureData = ImageTools::LoadCompressedTextureDataFromDDS(filepath);
    else
        textureData = ImageTools::LoadUncompressedTextureData(filepath);
    if (!textureData.data) {
        HZ_CORE_ERROR("[TextureLoader] Failed to load: {}, Data is null!", filepath);
        return nullptr;
    }

	std::unique_ptr<Moon::Rendering::Texture> texture = std::make_unique<Moon::Rendering::Texture>(Moon::Rendering::ETextureType::TEXTURE_2D);
    texture->Create(textureData);
    texture->Upload(textureData.data, textureData.format);
    Moon::Assets::Texture* asset = new Moon::Assets::Texture{ filepath, std::move(texture) };
    s_textures[filepath] = asset;
    FreeTextureData(textureData);
    return asset;
}

void Moon::Loaders::TextureLoader::FreeTextureData(Moon::Rendering::Data::TextureData& data) {
    if (!data.data) return;
    if (data.imageDataType == Moon::Rendering::Data::ImageDataType::UNCOMPRESSED)
        stbi_image_free(data.data);
    else
        delete[] static_cast<char*>(data.data);
    data.data = nullptr;
}
