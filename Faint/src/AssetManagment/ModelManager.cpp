#include "ModelManager.h"
#include "MaterialManager.h"

Moon::Model* Moon::AssetManagment::ModelManager::CreateResource(const std::string& p_path) {
    Model* model = Loaders::ModelLoader::Create(p_path, false);
    if (model)
        *reinterpret_cast<std::string*>(reinterpret_cast<char*>(model) + offsetof(Model, path)) = p_path; // Force the resource path to fit the given path
    return model;
}

void Moon::AssetManagment::ModelManager::DestroyResource(Model* p_resource) {
    Loaders::ModelLoader::Destroy(p_resource);
}

void Moon::AssetManagment::ModelManager::ReloadResource(Model* p_resource, const std::string& p_path) {
    Loaders::ModelLoader::Reload(*p_resource, p_path);
}
