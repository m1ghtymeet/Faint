#pragma once
#include "AssetManagment/Serializable.h"
#include <Types/Renderer/Mesh.h>

#include <vector>
#include <unordered_map>

namespace Moon {
	namespace Loaders { class ModelLoader; }
	namespace Rendering { class GeometryRenderPass; }

	struct MeshData {
		std::string name;
		std::vector<Moon::Rendering::Vertex> vertices;
		std::vector<uint32_t> indices;
		glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		uint32_t parentIndex = -1;
		uint32_t materialIndex = 0;
	};

	struct ModelData {
		std::string name;
		uint32_t meshCount;
		uint64_t timestamp;
		std::vector<MeshData> meshes;
		std::vector<std::string> materialNames;
		glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
	};

	class Model {
	public:
		Model() = default;
		~Model();

		void AddMeshIndex(uint32_t index);

		const size_t GetMeshCount() const;
		const std::vector<uint32_t>& GetMeshIndices() const;
		std::vector<std::string>& GetMaterialNames();

		std::string path;
		ModelData m_modelData;
	private:
		std::vector<uint32_t> m_meshIndices;
		friend class Loaders::ModelLoader;
		friend class Rendering::GeometryRenderPass;
	};
}