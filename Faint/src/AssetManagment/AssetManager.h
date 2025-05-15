#pragma once

#include <string>
#include "Core/Base.h"
#include "Common/UUID.h"
#include "Common/Types.h"
#include "Renderer/Types/Mesh.h"
#include "Renderer/Types/Model.h"

namespace Faint { class Material; class Texture; }
namespace FaintUI { class Font; }

using namespace Faint;
using namespace FaintUI;

namespace AssetManager {

	std::map<Faint::UUID, std::string>& GetAssets();
	std::vector<std::string>& GetLoadLog();

	bool IsAssetLoaded(const Faint::UUID& uuid);

	// Model
	Ref<Model> LoadModel(const std::string& path);
	
	// Texture
	Ref<Texture> LoadTexture(const std::string& path);

	// Material
	Ref<Material> LoadMaterial(const std::string& path);

	Ref<Font> LoadFont(const std::string& font);
}