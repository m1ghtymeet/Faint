#include "hzpch.h"
#include "Model.h"
#include "Renderer/Types/Mesh.h"
#include "AssetManagment/ModelLoader.h"

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

	bool Model::SerializeDataYaml(YAML::Emitter& out)
	{
		out << YAML::Key << "Model" << YAML::Value;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "UUID" << YAML::Value << static_cast<uint64_t>(id);
			out << YAML::Key << "Path" << YAML::Value << Path;
			for (uint32_t i = 0; i < std::size(m_meshes); i++) {
				out << YAML::Key << "Mesh_" + std::to_string(i) << YAML::Value;
				m_meshes[i]->SerializeYaml(out);
			}
		}
		out << YAML::EndMap;

		return true;
	}

	bool Model::DeserializeDataYaml()
	{
		/*if (j.contains("Path")) {
			this->IsEmbedded = true;

			ModelLoader loader;
			auto otherModel = loader.LoadModel(j["Path"], false);
			m_meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else {
			if (j.contains("UUID"))
			{
				id = UUID(j["UUID"]);
			}

			if (j.contains("Meshes"))
			{
				for (auto& m : j["Meshes"])
				{
					Ref<Mesh> mesh = CreateRef<Mesh>();
					mesh->Deserialize(m);

					m_meshes.push_back(mesh);
				}
			}
		}*/
		return true;
	}

	json Model::Serialize() {

		BEGIN_SERIALIZE();

		if (this->Path != "") {
			j["Path"] = this->Path;
		}
		else {
			j["UUID"] = static_cast<uint64_t>(id);
			for (uint32_t i = 0; i < std::size(m_meshes); i++) {
				j["Meshes"][i] = m_meshes[i]->Serialize();
			}
		}

		END_SERIALIZE();
	}

	bool Model::Deserialize(const json& j) {

		if (j.contains("Path")) {
			this->IsEmbedded = true;

			ModelLoader loader;
			auto otherModel = loader.LoadModel(j["Path"], false);
			m_meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else {
			if (j.contains("UUID"))
			{
				id = UUID(j["UUID"]);
			}

			if (j.contains("Meshes"))
			{
				for (auto& m : j["Meshes"])
				{
					Ref<Mesh> mesh = CreateRef<Mesh>();
					mesh->Deserialize(m);

					m_meshes.push_back(mesh);
				}
			}
		}
		return true;
	}
}