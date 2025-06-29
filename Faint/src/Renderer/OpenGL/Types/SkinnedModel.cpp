#include "SkinnedModel.h"

namespace Faint {
	void SkinnedModel::AddMesh(SkinnedMesh mesh) {
		m_meshes.push_back(mesh);
	}

	bool SkinnedModel::BoneExists(const std::string& boneName) {
		return m_boneMapping.find(boneName) != m_boneMapping.end();
	}

	std::vector<SkinnedMesh>& SkinnedModel::GetMeshes() {
		return m_meshes;
	}
	json SkinnedModel::Serialize() {
		BEGIN_SERIALIZE();
		//j["UUID"] = static_cast<uint64_t>(id);
		for (uint32_t i = 0; i < std::size(m_meshes); i++) {
			j["Meshes"][i] = m_meshes[i].Serialize();
		}
		END_SERIALIZE();
	}

	void SkinnedModel::Deserialize(const json& j) {

	}
}