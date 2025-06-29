#include "hzpch.h"
#include "Model.h"
#include "Renderer/OpenGL/Types/Mesh.h"
#include "AssetManagment/AssetManager.h"

namespace Faint {
	Model::Model() : m_meshes(std::vector<Ref<Mesh>>())
	{ }

	Model::Model(const std::string path) : m_meshes(std::vector<Ref<Mesh>>()) {
		this->Path = path;
	}
	void Model::AddMesh(Ref<Mesh> mesh) {
		m_meshes.push_back(mesh);
	}

	std::vector<Ref<Mesh>>& Model::GetMeshes() {
		return m_meshes;
	}

	json Model::SerializeData() {

		BEGIN_SERIALIZE();
		j["UUID"] = static_cast<uint64_t>(id);
		for (uint32_t i = 0; i < std::size(m_meshes); i++) {
			j["Meshes"][i] = m_meshes[i]->Serialize();
		}
		END_SERIALIZE();
	}

	json Model::Serialize() {
		BEGIN_SERIALIZE();
		if (this->Path != "") {
			j["ID"] = static_cast<uint64_t>(id);
			for (uint32_t i = 0; i < std::size(m_meshes); i++) {
				j["Meshes"][i] = m_meshes[i]->Serialize();
			}
		}
		END_SERIALIZE();
	}

	void Model::Deserialize(const json& j) {
		if (j.contains("Path")) {
			this->IsEmbedded = true;
			Model* otherModel = AssetManager::LoadModel(j["Path"], false);
			m_meshes = otherModel->GetMeshes();
			this->Path = j["Path"];
		}
		else {
			if (j.contains("UUID")) {
				id = UUID(j["UUID"]);
			}

			if (j.contains("Meshes")) {
				for (auto& m : j["Meshes"]) {
					Ref<Mesh> mesh = CreateRef<Mesh>();
					mesh->Deserialize(m);

					m_meshes.push_back(mesh);
				}
			}
		}
	}
}