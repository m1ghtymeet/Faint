#pragma once
#include "Common/Types.h"
#include <glad/glad.h>
#include <memory>

namespace Faint {
	struct OpenGLTexture {
	public:
		OpenGLTexture() = default;
		~OpenGLTexture();
		GLuint& GetHandle();
		GLuint64 GetBindlessID();
		void Create(int width, int height, int format, int internalFormat, int mipmapLevelCount, void* data);
		void AllocateMemory(int width, int height, int format, int internalFormat, int mipmapLevelCount);
		void SetWrapMode(TextureWrapMode wrapMode);
		void SetMinFilter(TextureFilter filter);
		void SetMagFilter(TextureFilter filter);
		int GetWidth();
		int GetHeight();
		int GetChannelCount();
		void* GetData();
		GLint GetFormat();
		GLint GetInternalFormat();
		GLint GetMipmapLevelCount();
	private:
		GLuint m_handle = 0;
		GLuint64 m_bindlessID = 0;
		int m_width = 0;
		int m_height = 0;
		int m_channelCount = 0;
		GLsizei m_dataSize = 0;
		void* m_data = nullptr;
		GLenum m_format = 0;
		GLenum m_internalFormat = 0;
		GLenum m_mipmapLevelCount = 0;
		ImageDataType m_imageDataType;
		bool m_memoryAllocated = false;
	};
}