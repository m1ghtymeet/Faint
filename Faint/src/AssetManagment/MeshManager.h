#pragma once
#include <Types/Renderer/Mesh.h>
#include <Types/Renderer/SkinnedMesh.h>
#include <Renderer/Types/Vertex.h>
#include <Renderer/Types/WeightedVertex.h>

namespace Moon::AssetManagment {
	class MeshManager {
	public:
		static void Init();
		static void Cleanup();

		static uint32_t CreateMesh(
			const std::string& name,
			const std::vector<Rendering::Vertex>& vertices,
			const std::vector<uint32_t>& indices,
			uint32_t materialIndex,
			glm::vec3 aabbMin, glm::vec3 aabbMax);

		static uint32_t CreateMesh(
			const std::string& name,
			const std::vector<Rendering::Vertex>& vertices,
			const std::vector<uint32_t>& indices,
			uint32_t materialIndex = 0
		);

		static Mesh* GetMeshByIndex(uint32_t id);
		static Mesh* GetMeshByName(const std::string& name);
		static uint32_t GetMeshIndexByName(const std::string& name);

		static Mesh* GetQuad();

		static uint32_t CreateSkinnedMesh(
			const std::string& name,
			const std::vector<Rendering::WeightedVertex>& vertices,
			const std::vector<uint32_t>& indices,
			uint32_t baseVertexLocal,
			glm::vec3 aabbMin, glm::vec3 aabbMax,
			uint32_t materialIndex = 0
		);

		// Vertex Data
		static std::vector<Rendering::Vertex>& GetVertices();
		static std::vector<uint32_t>& GetIndices();

	private:
		inline static std::vector<Mesh> m_meshes;
		inline static std::vector<Rendering::Vertex> m_vertices;
		inline static std::vector<uint32_t> m_indices;

		inline static std::vector<Rendering::SkinnedMesh> m_skinnedMeshes;
		inline static std::vector<Rendering::WeightedVertex> m_skinnedVertices;
		inline static std::vector<uint32_t> m_skinnedIndices;

		static uint32_t g_nextVertexInsert;
		static uint32_t g_nextIndexInsert;

		static uint32_t g_nextWeightedVertexInsert;
		static uint32_t g_nextWeightedIndexInsert;
	};
}