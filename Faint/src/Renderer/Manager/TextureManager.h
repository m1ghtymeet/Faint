#pragma once
#include "Core/Base.h"

namespace Faint {

	class Texture;

	// TODO: SHOULD probably be static too.
	class TextureManager {
	private:
		static TextureManager* s_Instance;

		static std::map<std::string, Ref<Texture>> m_Registry;
		bool IsTextureLoaded(const std::string path);

		void LoadStaticTextures();

	public:
		static TextureManager* Get();

		TextureManager();

		Ref<Texture> GetTexture(const std::string path);
	};
}