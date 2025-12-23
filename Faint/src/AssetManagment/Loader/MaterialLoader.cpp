#include "MaterialLoader.h"
#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>
#include <fstream>
#include <iostream>

Moon::Rendering::Material* Moon::Loaders::MaterialLoader::Create(const std::string& p_filepath, bool p_absolute) {

    if (!FileSystem::FileExists(p_filepath, p_absolute))
        std::cout << "[MATERIAL] '" << p_filepath << "' Not Found!\n";
    std::string content = FileSystem::ReadFile(p_filepath, p_absolute);
    json j = json::parse(content);

    Rendering::Material* material = new Rendering::Material();
    material->path = p_filepath;
    material->Deserialize(j);
    return material;
}

void Moon::Loaders::MaterialLoader::Reload(Moon::Rendering::Material& p_material, const std::string& p_path) {

    std::string content = FileSystem::ReadFile(p_path, true);
    json j = json::parse(content);

    if (FileSystem::FileExists(p_path, true)) {
        Rendering::Material* material = new Rendering::Material();
        material->path = p_path;
        material->Deserialize(j);
        HZ_CORE_INFO("[MATERIAL] \"" + p_path + "\" Reloaded");
    }
}

void Moon::Loaders::MaterialLoader::Save(Moon::Rendering::Material& p_material, const std::string& p_path) {

    json j = p_material.Serialize();
    std::string out = j.dump(4);
    std::ofstream fout(p_path);
    fout << out.c_str();
}