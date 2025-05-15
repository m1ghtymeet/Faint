#include "MaterialManager.h"

#include <fstream>
#include <streambuf>

#include "Renderer/Types/Material.h"
#include "nlohmann/json.hpp"

namespace Faint {

	using json = nlohmann::json;

	Ref<MaterialManager> MaterialManager::s_Instance;

	bool MaterialManager::IsMaterialLoaded(const std::string path) {
		return m_Materials.find(path) != m_Materials.end();
	}

	MaterialManager::MaterialManager() {

		m_Materials = std::map<std::string, Ref<Material>>();

		s_Instance = CreateRef<MaterialManager>();

		GetMaterial(DEFAULT_MATERIAL);
	}

	Ref<Material> MaterialManager::GetMaterial(const std::string name) {
		if (!IsMaterialLoaded(name)) {
			Ref<Material> newMaterial = CreateRef<Material>(name);
			RegisterMaterial(newMaterial);
			return newMaterial;
		}
		return m_Materials[name];
	}

	Ref<MaterialManager> MaterialManager::Get() {
		return s_Instance;
	}

	Ref<Material> MaterialManager::LoadMaterial(const std::string& path)
	{
		if (IsMaterialLoaded(path))
			return (m_Materials)[path];

		std::string finalPath = path + ".material";
		std::ifstream i(finalPath);
		json j;
		i >> j;

		std::string matName;
		if (!j.contains("name"))
			return nullptr;
		else
			matName = j["name"];

		std::string albedoPath;
		std::string normalPath;
		std::string aoPath;
		std::string metalPath;
		std::string roughnessPath;
		std::string displacementPath;

		Ref<Material> newMaterial = CreateRef<Material>(j["albedo"]);
		newMaterial->SetName(matName);

		return newMaterial;
	}

	void MaterialManager::RegisterMaterial(Ref<Material> material) {
		m_Materials[material->GetName()] = material;
	}
}
