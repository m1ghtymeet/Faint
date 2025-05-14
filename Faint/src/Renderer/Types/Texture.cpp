#include "hzpch.h"
#include "Texture.h"
#include "../Renderer.h"

#include "FileSystem/FileSystem.h"

#include <stb_image.h>

namespace Faint {

	Texture::Texture(const std::string& path, bool flip)
		: m_path(path) {

		_ID = 0;

		int width, height, channels;
		stbi_set_flip_vertically_on_load(flip);
		void* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image!");
		m_width = width;
		m_height = height;

		glGenTextures(1, &_ID);
		glBindTexture(GL_TEXTURE_2D, _ID);

		GLint format = GL_RGB;
		if (channels == 4)
			format = GL_RGBA;
		if (channels == 1)
			format = GL_RED;
		HZ_CORE_ASSERT(format, "Format not supported!");

		glTextureParameteri(_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTextureParameteri(_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		// glTextureSubImage2D(_ID, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
	}

	Texture::Texture(Vec2 size, GLenum format, GLenum format2, GLenum format3, void* data)
	{
		_ID = 0;
		m_Format = format;

		m_Format2 = format;
		if (format2 != 0)
			m_Format2 = format2;

		m_Filtering = GL_LINEAR;

		m_Format3 = GL_UNSIGNED_BYTE;
		if (format3 != 0)
			m_Format3 = format3;

		m_width = size.x;
		m_height = size.y;

		glGenTextures(1, &_ID);
		glBindTexture(GL_TEXTURE_2D, _ID);
		glTexImage2D(GL_TEXTURE_2D, 0, format2, size.x, size.y, 0, format, format3, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture::~Texture() {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &_ID);
	}
	void Texture::Bind(int slot) const {
		//glBindTextureUnit(slot, _ID);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, _ID);
	}
	void Texture::UnBind() const {
		//glBindTextureUnit(0, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void Texture::AttachToFramebuffer(GLenum attachment)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, _ID, 0);
	}
	void Texture::Resize(Vec2 size)
	{
		glDeleteTextures(1, &_ID);
		m_width = size.x;
		m_height = size.y;
		glGenTextures(1, &_ID);
		glBindTexture(GL_TEXTURE_2D, _ID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_Format2, size.x, size.y, 0, m_Format, m_Format3, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Filtering);
	}
	void Texture::SetParameter(const GLenum& param, const GLenum& value)
	{
		if (param == GL_TEXTURE_MIN_FILTER) {
			m_Filtering = value;
		}

		Bind();
		glTexParameteri(GL_TEXTURE_2D, param, value);
		UnBind();
	}

	json Texture::Serialize()
	{
		BEGIN_SERIALIZE();
		const std::string& relativePath = FileSystem::AbsoluteToRelative(m_path);
		j["Path"] = relativePath;
		END_SERIALIZE();
	}

	bool Texture::Deserialize(const json& j)
	{
		if (j.contains("Path"))
			return false;
		return true;
	}
}