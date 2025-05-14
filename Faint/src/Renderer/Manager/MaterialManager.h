#pragma once
#include "Core/Base.h"
#include "Common/UUID.h"

namespace Faint {

	class Material;

	class MaterialManager {
	private:
		static Ref<MaterialManager> s_Instance;

		std::map<std::string, Ref<Material>> m_Materials;

		// TODO: Pile of crap
		Material* ParseMaterialFile(const std::string path);
		void SaveMaterialFile(const std::string path, Material* material);
		bool IsMaterialLoaded(const std::string path);

	public:
		const std::string DEFAULT_MATERIAL = "data/textures/Default.png";
		MaterialManager();

		void RegisterMaterial(Ref<Material> material);

		Ref<Material> LoadMaterial(const std::string& path);
		Ref<Material> GetMaterial(const std::string name);
		
		static Ref<MaterialManager> Get();

		friend class Material;
	};
}