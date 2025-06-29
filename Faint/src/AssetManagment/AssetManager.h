#pragma once
#include "Core/Base.h"
#include "Common/UUID.h"
#include "Common/Types.h"
#include "Renderer/OpenGL/Types/Mesh.h"
#include "Renderer/OpenGL/Types/Model.h"
#include "Renderer/OpenGL/Types/SkinnedMesh.h"
#include "Renderer/OpenGL/Types/SkinnedModel.h"
#include "Renderer/Types/Texture.h"

namespace Faint { class Material; }

using namespace Faint;

namespace AssetManager {

	std::map<Faint::UUID, std::string>& GetAssets();
	std::vector<std::string>& GetLoadLog();

	bool IsAssetLoaded(const Faint::UUID& uuid);

	// Models
	Model* LoadModel(const std::string& path, bool absolute = false);
	Model* GetModelByName(const std::string& name);

	// Skinned Models
	Ref<SkinnedModel> LoadSkinnedModel(const std::string& path, bool absolute = false);

	// Textures
	Ref<Texture> LoadTexture(const std::string& path);
	Texture* GetTextureByIndex(int index);

	// Materials
	Ref<Material> LoadMaterial(const std::string& path);
}