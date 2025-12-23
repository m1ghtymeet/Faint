#include "ModelLoader.h"
#include <AssetManagment/MeshManager.h>
#include <FileSystem/FileSystem.h>
#include <Core/String.h>
#include <Util/ModelTools.h>
#include <Util/AssimpImporter.h>
#include <Util/BlendImporter.h>
#include <iostream>
#include <fstream>

Moon::Model* Moon::Loaders::ModelLoader::Create(const std::string& p_filepath, bool p_absolute, bool needToCache) {
	Model* result = new Model();
	result->path = p_filepath;

	if (!FileSystem::FileExists(p_filepath, true))
		std::cout << "[MODEL] '" << p_filepath << "' Not Found!\n";

	const std::string cachePath = FileSystem::GetParentPath(p_filepath) + std::filesystem::path(p_filepath).stem().string() + ".model";
	{
		if (FileSystem::FileExists(cachePath, true))
			result->m_modelData = ModelTools::ImportModel(cachePath);
		else {
			if (String::EndsWith(p_filepath, "blend")) {
				result->m_modelData = BlendImporter::ImportBlend(p_absolute ? FileSystem::Root + p_filepath : p_filepath);
			}
			else {
				result->m_modelData = AssimpImporter::ImportFbx(p_absolute ? FileSystem::Root + p_filepath : p_filepath);
				if (needToCache && !FileSystem::FileExists(cachePath, true))
					ModelTools::ExportModel(result->m_modelData, cachePath);
			}
		}

		for (auto& meshData : result->m_modelData.meshes) {
			int meshIndex = AssetManagment::MeshManager::CreateMesh(meshData.name, meshData.vertices, meshData.indices, meshData.materialIndex);
			result->AddMeshIndex(meshIndex);
		}
		return result;
	}

	delete result;
    return nullptr;
}

void Moon::Loaders::ModelLoader::Reload(Model& p_model, const std::string& p_filepath) {

	Model* newModel = Create(p_filepath);
	
	if (newModel) {
		p_model.m_modelData.meshes = newModel->m_modelData.meshes;
		p_model.m_modelData.materialNames = newModel->m_modelData.materialNames;
		newModel->m_modelData.meshes.clear();
		delete newModel;
	}
}

bool Moon::Loaders::ModelLoader::Destroy(Model*& p_modelInstance) {
	if (p_modelInstance) {
		delete p_modelInstance;
		p_modelInstance = nullptr;

		return true;
	}
	return false;
}

Moon::Rendering::SkinnedModel* Moon::Loaders::SkinnedModelLoader::Create(const std::string& p_filepath, bool p_absolute, bool needToCache) {
	Moon::Rendering::SkinnedModel* result = new Moon::Rendering::SkinnedModel();

	if (!FileSystem::FileExists(p_filepath, true))
		std::cout << "[SKINMODEL] '" << p_filepath << "' Not Found!\n";

	Moon::Rendering::SkinnedModelData modelData = AssimpImporter::ImportSkinnedFbx(p_filepath);
	for (auto& meshData : modelData.meshes) {
		int meshIndex = AssetManagment::MeshManager::CreateSkinnedMesh(meshData.name, meshData.vertices,
			meshData.indices, meshData.localBaseVertex, meshData.aabbMin,
			meshData.aabbMax, meshData.materialIndex);
		result->AddMeshIndex(meshIndex);
	}
	return result;

	delete result;
	return nullptr;
}
