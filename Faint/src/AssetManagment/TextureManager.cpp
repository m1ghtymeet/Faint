#include "TextureManager.h"

Moon::Assets::Texture* Moon::AssetManagment::TextureManager::CreateResource(const std::string& p_path) {
    Assets::Texture* texture = Moon::Loaders::TextureLoader::Create(p_path);
    
    if (texture)
        *reinterpret_cast<std::string*>(reinterpret_cast<char*>(texture) + offsetof(Assets::Texture, path)) = p_path;
    
    return texture;
}

void Moon::AssetManagment::TextureManager::DestroyResource(Assets::Texture* p_resource) {

}

void Moon::AssetManagment::TextureManager::ReloadResource(Assets::Texture* p_resource, const std::string& p_path) {

}
