#pragma once

#include "Core/Base.h"
#include "Math/Math.h"
#include <glad/glad.h>

#include "AssetManagment/Serializable.h"

#include <string>

namespace Faint {

	typedef unsigned int GLenum;

	class Texture : ISerializable {
	public:
		Texture(const std::string& path, bool flip = true);
		Texture(Vec2 size, GLenum format, GLenum format2 = 0, GLenum format3 = 0, void* data = 0);
		~Texture();
		
		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }
		uint32_t GetID() { return _ID; }

		void Bind(int slot = 0) const;
		void UnBind() const;
		
		void AttachToFramebuffer(GLenum attachment);
		void Resize(Vec2 size);
		void SetParameter(const GLenum& param, const GLenum& value);

		json Serialize() override;
		bool Deserialize(const json& j) override;
	private:
		unsigned int _ID;
		std::string m_path;
		unsigned int m_width;
		unsigned int m_height;
		GLenum m_Format;
		GLenum m_Format2;
		GLenum m_Format3;
		GLenum m_Filtering;
	};
}