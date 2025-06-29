#pragma once
#include "Renderer/OpenGL/Types/GL_texture.h"
#include "AssetManagment/Serializable.h"

namespace Faint {
	struct Texture : public ISerializable {
	public:
		Texture(const std::string& filepath);
		~Texture() = default;
		void Load(const std::string& filepath);
		void FreeCPUMemory();
		GLuint GetID() { return m_texture.GetHandle(); }
		OpenGLTexture& GetTexture();
		json Serialize() override;
		void Deserialize(const json& j) override;
	private:
		OpenGLTexture m_texture;
		ImageDataType m_imageDataType = ImageDataType::UNDEFINED;
		std::string m_path;
		std::vector<TextureData> m_textureDataLevels;
		bool m_bakeComplate = false;
	};
}