#include "TextureManager.h"

#include "Renderer/Types/Texture.h"

namespace Faint {

	std::map<std::string, Ref<Texture>> TextureManager::m_Registry;

	TextureManager* TextureManager::s_Instance = nullptr;

	bool TextureManager::IsTextureLoaded(const std::string path) {
		return m_Registry.find(path) != m_Registry.end();
	}

	TextureManager* TextureManager::Get() {
		return s_Instance;
	}

	TextureManager::TextureManager() {

		m_Registry.emplace("data/textures/Default.png", CreateRef<Texture>("data/textures/Default.png"));
	}

	Ref<Texture> TextureManager::GetTexture(const std::string path) {
		if (!IsTextureLoaded(path))
			m_Registry.emplace(path, CreateRef<Texture>(path));
		return m_Registry.at(path);
	}
}
