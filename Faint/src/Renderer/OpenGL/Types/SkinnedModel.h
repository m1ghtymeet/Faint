#pragma once
#include "SkinnedMesh.h"

namespace Faint {
	struct SkinnedModel : public ISerializable {
	public:
		SkinnedModel() = default;

		void AddMesh(SkinnedMesh mesh);
		bool BoneExists(const std::string& boneName);

		std::vector<SkinnedMesh>& GetMeshes();

		json Serialize() override;
		void Deserialize(const json& j) override;
	public:
		std::vector<Node> m_nodes;
		std::vector<glm::mat4> m_boneOffsets;
		std::map<std::string, unsigned int> m_boneMapping;
	private:
		std::vector<SkinnedMesh> m_meshes;
	};
}