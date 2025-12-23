#pragma once
#include "SkinnedMesh.h"
#include <Renderer/Types/WeightedVertex.h>
#include <map>
#include <vector>

namespace Moon::Rendering {
	struct Node {
		std::string name;
		int parentIndex;
		glm::mat4 inverseBindTransform;
	};

	struct SkinnedMeshData {
		std::string name;
		std::vector<WeightedVertex> vertices;
		std::vector<uint32_t> indices;
		glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
		uint32_t materialIndex;
		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t localBaseVertex;
	};

	struct SkinnedModelData {
		std::string name;
		std::vector<SkinnedMeshData> meshes;
		std::vector<glm::mat4> boneOffsets;
		std::vector<Node> nodes;
		std::map<std::string, unsigned int> boneMapping;
		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t timestamp;
	};

	class SkinnedModel {
	public:
		SkinnedModel() = default;

		void AddMeshIndex(uint32_t index);
		std::vector<uint32_t>& GetMeshIndices();
		uint32_t GetMeshCount();
		uint32_t GetVertexCount();
		uint32_t GetBoneCount();
		uint32_t GetNodeCount();

	public:
		std::vector<Node> m_nodes;
		std::vector<glm::mat4> m_boneOffsets;
		std::map<std::string, unsigned int> m_boneMapping;
		std::map<std::string, unsigned int> m_nodeMapping;
		std::vector<int> m_boneNodeIndices;
		SkinnedModelData m_modelData;

	private:
		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
		std::vector<uint32_t> m_meshIndices;
	};
}