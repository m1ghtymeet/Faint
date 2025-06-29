#include "Texture.h"
#include "Renderer/Renderer.h"
#include "FileSystem/FileSystem.h"

#include <stb_image.h>

namespace Faint {
	GLint GetFormatFromChannelCount(int channelCount) {
		switch (channelCount) {
		case 4: return GL_RGBA;
		case 3: return GL_RGB;
		case 1: return GL_RED;
		default:
			HZ_CORE_ASSERT("Unsupported channel count: " + channelCount);
			return -1;
		}
	}

	GLint GetInternalFormatFromChannelCount(int channelCount) {
		switch (channelCount) {
		case 4: return GL_RGBA8;
		case 3: return GL_RGB8;
		case 1: return GL_R8;
		default:
			HZ_CORE_ASSERT("Unsupported channel count: " + channelCount);
			return -1;
		}
	}

	TextureData LoadUncompressedTextureData(const std::string& filepath) {
		stbi_set_flip_vertically_on_load(true);
		TextureData textureData;
		uint8_t* imageData = stbi_load(filepath.c_str(), &textureData.m_width, &textureData.m_height, &textureData.m_numChannels, 0);
		textureData.m_imageDataType = ImageDataType::UNCOMPRESSED;
		if (!imageData || filepath == "") {
			imageData = stbi_load("data/textures/CheckerBoard.png", &textureData.m_width, &textureData.m_height, &textureData.m_numChannels, 0);
		}
		if (textureData.m_numChannels == 3) {
			size_t newSize = textureData.m_width * textureData.m_height * 4;
			uint8_t* rgbaData = new uint8_t[newSize];
			for (size_t i = 0, j = 0; i < newSize; i += 4, j += 3) {
				rgbaData[i] = imageData[j];			// R
				rgbaData[i + 1] = imageData[j + 1]; // G
				rgbaData[i + 2] = imageData[j + 2]; // B
				rgbaData[i + 3] = 255; // A
			}
			stbi_image_free(imageData);
			textureData.m_data = rgbaData;
			textureData.m_numChannels = 4;
		}
		else {
			textureData.m_data = imageData;
		}

		// If mipmaps are requested, allocate space for them
		if (textureData.m_numChannels == 4 && textureData.m_width != textureData.m_height) {
			textureData.m_dataSize = textureData.m_width * textureData.m_height * 4;
		}
		else {
			textureData.m_dataSize = textureData.m_width * textureData.m_height * textureData.m_numChannels;
		}
		textureData.m_format = GetFormatFromChannelCount(textureData.m_numChannels);
		textureData.m_internalFormat = GetInternalFormatFromChannelCount(textureData.m_numChannels);

		return textureData;
	}

	Texture::Texture(const std::string& filepath) {
		Load(filepath);
	}

	void Texture::Load(const std::string& filepath) {
		m_path = filepath;

		TextureData data = LoadUncompressedTextureData(filepath);
		if (Renderer::GetAPI() == RendererAPI::OPENGL) {
			m_texture.Create(data.m_width, data.m_height, data.m_format, data.m_internalFormat, 1 + static_cast<int>(std::log2(std::max(data.m_width, data.m_height))), data.m_data);
			m_texture.SetMagFilter(TextureFilter::LINEAR);
			m_texture.SetMinFilter(TextureFilter::LINEAR_MIPMAP);
			m_texture.SetWrapMode(TextureWrapMode::CLAMP_TO_EDGE);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			HZ_CORE_ERROR("API IS NULL!");
		}
	}

	void Texture::FreeCPUMemory() {
		
	}

	OpenGLTexture& Texture::GetTexture() {
		return m_texture;
	}

	json Texture::Serialize() {
		BEGIN_SERIALIZE();
		j["Path"] = FileSystem::AbsoluteToRelative(m_path);
		END_SERIALIZE();
	}

	void Texture::Deserialize(const json& j) {

	}
}