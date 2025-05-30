#pragma once
#include "Core/Base.h"
#include "Renderer/Types/Texture.h"

namespace Faint {

	// TODO: SHOULD probably be static too.
	class TextureManager {
	private:
		static TextureManager* s_Instance;

		static std::map<std::string, Ref<Texture>> m_Registry;
		bool IsTextureLoaded(const std::string path);

	public:
		static TextureManager* Get();

		TextureManager();

		Ref<Texture> GetTexture(const std::string path);
	};
}