#include "GL_texture.h"

namespace OpenGLUtil {
	GLint TextureWrapModeToGLEnum(TextureWrapMode wrapMode) {
		switch (wrapMode) {
		case TextureWrapMode::REPEAT: return GL_REPEAT;
		case TextureWrapMode::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case TextureWrapMode::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		case TextureWrapMode::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
		default: return GL_NONE;
		}
	}
	GLint TextureFilterToGLEnum(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::NEAREST: return GL_NEAREST;
		case TextureFilter::LINEAR: return GL_LINEAR;
		case TextureFilter::LINEAR_MIPMAP: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_NONE;
		}
	}
}

namespace Faint {
	OpenGLTexture::~OpenGLTexture() {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &m_handle);
	}
	GLuint& OpenGLTexture::GetHandle() {
		return m_handle;
	}
	GLuint64 OpenGLTexture::GetBindlessID() {
		return m_bindlessID;
	}
	void OpenGLTexture::Create(int width, int height, int format, int internalFormat, int mipmapLevelCount, void* data) {
		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
		glTextureStorage2D(m_handle, mipmapLevelCount, internalFormat, width, height);
		glBindTexture(GL_TEXTURE_2D, m_handle);
		m_width = width;
		m_height = height;
		int mipmapWidth = width;
		int mipmapHeight = height;
		for (int i = 0; i < mipmapLevelCount; i++) {
			//if (m_imageDataType == ImageDataType::UNCOMPRESSED) {
				glTextureSubImage2D(m_handle, i, 0, 0, mipmapWidth, mipmapHeight, format, GL_UNSIGNED_BYTE, data);
			//}
			mipmapWidth = std::max(1, mipmapWidth / 2);
			mipmapHeight = std::max(1, mipmapHeight / 2);
		}
	}
	void OpenGLTexture::AllocateMemory(int width, int height, int format, int internalFormat, int mipmapLevelCount) {
		if (m_memoryAllocated)
			return;
		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
		glTextureStorage2D(m_handle, mipmapLevelCount, internalFormat, width, height);
		m_width = width;
		m_height = height;
		m_mipmapLevelCount = mipmapLevelCount;
		glBindTexture(GL_TEXTURE_2D, m_handle);
		int mipmapWidth = width;
		int mipmapHeight = height;
		for (int i = 0; i < mipmapLevelCount; i++) {
			if (m_imageDataType == ImageDataType::UNCOMPRESSED) {
				glTextureSubImage2D(m_handle, i, 0, 0, mipmapWidth, mipmapHeight, format, GL_UNSIGNED_BYTE, nullptr);
			}
			if (m_imageDataType == ImageDataType::COMPRESSED) {
				glCompressedTextureSubImage2D(m_handle, i, 0, 0, mipmapWidth, mipmapHeight, internalFormat, 0, nullptr);
			}
			mipmapWidth = std::max(1, mipmapWidth / 2);
			mipmapHeight = std::max(1, mipmapHeight / 2);
		}
		m_memoryAllocated = true;
	}
	void OpenGLTexture::SetWrapMode(TextureWrapMode wrapMode) {
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, OpenGLUtil::TextureWrapModeToGLEnum(wrapMode));
		glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, OpenGLUtil::TextureWrapModeToGLEnum(wrapMode));
	}
	void OpenGLTexture::SetMinFilter(TextureFilter filter) {
		glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, OpenGLUtil::TextureFilterToGLEnum(filter));
	}
	void OpenGLTexture::SetMagFilter(TextureFilter filter) {
		glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, OpenGLUtil::TextureFilterToGLEnum(filter));
	}
	int OpenGLTexture::GetWidth() {
		return m_width;
	}
	int OpenGLTexture::GetHeight() {
		return m_height;
	}
	int OpenGLTexture::GetChannelCount() {
		return m_channelCount;
	}
	void* OpenGLTexture::GetData() {
		return m_data;
	}
	GLint OpenGLTexture::GetFormat() {
		return m_format;
	}
	GLint OpenGLTexture::GetInternalFormat() {
		return m_internalFormat;
	}
	GLint OpenGLTexture::GetMipmapLevelCount() {
		return m_mipmapLevelCount;
	}
}