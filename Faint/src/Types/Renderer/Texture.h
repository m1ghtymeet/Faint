#pragma once
#include <Renderer/OpenGL/GL_Texture.h>
#include <memory>
#include <string>

namespace Moon::Rendering {
	using Texture = OpenGLTexture;
}

namespace Moon::Assets {
	namespace Loaders { class TextureLoader; }

	/**
	* Texture saved on the disk
	*/
	class Texture {
		friend class Loaders::TextureLoader;

	public:
		Texture(const std::string p_path, std::unique_ptr<Rendering::Texture>&& p_texture);
		~Texture() = default;
		
		void Load(const std::string p_path);

		void Bind(uint16_t slot) const;

		void SetTexture(std::unique_ptr<Rendering::Texture>&& p_texture);

		void FreeCPUMemory();

	private:
		std::unique_ptr<Rendering::Texture> m_texture;
	public:
		Rendering::Texture& GetTexture();

		std::string path;
	};
}