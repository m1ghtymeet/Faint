#include "MeshManager.h"
#include <vector>

#include <Debug/Log.h>
#include <Types/Renderer/Model.h>
#include <Types/Renderer/SkinnedModel.h>
#include <Renderer/OpenGL/GL_BackEnd.h>

uint32_t Moon::AssetManagment::MeshManager::g_nextVertexInsert;
uint32_t Moon::AssetManagment::MeshManager::g_nextIndexInsert;

uint32_t Moon::AssetManagment::MeshManager::g_nextWeightedVertexInsert;
uint32_t Moon::AssetManagment::MeshManager::g_nextWeightedIndexInsert;

void Moon::AssetManagment::MeshManager::Init() {
	Rendering::OpenGLBackEnd::InitGlobalBuffers();
	
	std::vector<Rendering::Vertex> vertices = {
		// Position            UV            Normal              Tangent
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-left
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-right
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}, // Top-right
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}  // Top-left
	};

	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	int meshIndex = CreateMesh("Quad", vertices, indices);
	
	/* Cube */ {
		std::vector<Rendering::Vertex> cubeVertices = {
			// FRONT FACE
			{{-0.5f, -0.5f,  0.5f}, {0, 0}, { 0, 0, 1}, {1, 0, 0}},
			{{ 0.5f, -0.5f,  0.5f}, {1, 0}, { 0, 0, 1}, {1, 0, 0}},
			{{ 0.5f,  0.5f,  0.5f}, {1, 1}, { 0, 0, 1}, {1, 0, 0}},
			{{-0.5f,  0.5f,  0.5f}, {0, 1}, { 0, 0, 1}, {1, 0, 0}},

			// BACK FACE
			{{ 0.5f, -0.5f, -0.5f}, {0, 0}, { 0, 0, -1}, {-1, 0, 0}},
			{{-0.5f, -0.5f, -0.5f}, {1, 0}, { 0, 0, -1}, {-1, 0, 0}},
			{{-0.5f,  0.5f, -0.5f}, {1, 1}, { 0, 0, -1}, {-1, 0, 0}},
			{{ 0.5f,  0.5f, -0.5f}, {0, 1}, { 0, 0, -1}, {-1, 0, 0}},

			// LEFT FACE
			{{-0.5f, -0.5f, -0.5f}, {0, 0}, {-1, 0, 0}, {0, 0, 1}},
			{{-0.5f, -0.5f,  0.5f}, {1, 0}, {-1, 0, 0}, {0, 0, 1}},
			{{-0.5f,  0.5f,  0.5f}, {1, 1}, {-1, 0, 0}, {0, 0, 1}},
			{{-0.5f,  0.5f, -0.5f}, {0, 1}, {-1, 0, 0}, {0, 0, 1}},

			// RIGHT FACE
			{{ 0.5f, -0.5f,  0.5f}, {0, 0}, { 1, 0, 0}, {0, 0, -1}},
			{{ 0.5f, -0.5f, -0.5f}, {1, 0}, { 1, 0, 0}, {0, 0, -1}},
			{{ 0.5f,  0.5f, -0.5f}, {1, 1}, { 1, 0, 0}, {0, 0, -1}},
			{{ 0.5f,  0.5f,  0.5f}, {0, 1}, { 1, 0, 0}, {0, 0, -1}},

			// TOP FACE
			{{-0.5f,  0.5f,  0.5f}, {0, 0}, { 0, 1, 0}, {1, 0, 0}},
			{{ 0.5f,  0.5f,  0.5f}, {1, 0}, { 0, 1, 0}, {1, 0, 0}},
			{{ 0.5f,  0.5f, -0.5f}, {1, 1}, { 0, 1, 0}, {1, 0, 0}},
			{{-0.5f,  0.5f, -0.5f}, {0, 1}, { 0, 1, 0}, {1, 0, 0}},

			// BOTTOM FACE
			{{-0.5f, -0.5f, -0.5f}, {0, 0}, { 0,-1, 0}, {1, 0, 0}},
			{{ 0.5f, -0.5f, -0.5f}, {1, 0}, { 0,-1, 0}, {1, 0, 0}},
			{{ 0.5f, -0.5f,  0.5f}, {1, 1}, { 0,-1, 0}, {1, 0, 0}},
			{{-0.5f, -0.5f,  0.5f}, {0, 1}, { 0,-1, 0}, {1, 0, 0}}
		};

		std::vector<uint32_t> cubeIndices = {
			0, 1, 2,  2, 3, 0,      // Front face
			4, 5, 6,  6, 7, 4,      // Back face
			8, 9, 10, 10, 11, 8,    // Left face
			12, 13, 14, 14, 15, 12, // Right face
			16, 17, 18, 18, 19, 16, // Top face
			20, 21, 22, 22, 23, 20  // Bottom face
		};

		int meshIndexCube = CreateMesh("Cube", cubeVertices, cubeIndices);
	}
}

void Moon::AssetManagment::MeshManager::Cleanup()
{
}

uint32_t Moon::AssetManagment::MeshManager::CreateMesh(const std::string& name, const std::vector<Rendering::Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex, glm::vec3 aabbMin, glm::vec3 aabbMax) {
	std::vector<Mesh>& meshes = m_meshes;
	std::vector<Rendering::Vertex>& allVertices = m_vertices;
	std::vector<uint32_t>& allIndices = m_indices;

	if (indices.empty())
		HZ_CORE_WARN("WARNING! Mesh {} has NO INDICES! Will be rendered wrong or not at all!");

	Mesh& mesh = meshes.emplace_back();
	mesh.baseVertex = static_cast<uint32_t>(m_vertices.size());
	mesh.firstIndex = static_cast<uint32_t>(m_indices.size());
	mesh.vertexCount = static_cast<uint32_t>(vertices.size());
	mesh.indexCount = static_cast<uint32_t>(indices.size());
	mesh.materialIndex = materialIndex;
	mesh.name = name;
	mesh.aabbMin = aabbMin;
	mesh.aabbMax = aabbMax;

	//HZ_CORE_TRACE("[DEBUG] Creating mesh: {}\n | Vertices: {}, Base: {}\n | Indices: {}, Base: {}",
	//	name, vertices.size(), mesh.baseVertex, indices.size(), mesh.firstIndex);

	allVertices.reserve(allVertices.size() + vertices.size());
	allVertices.insert(std::end(allVertices), std::begin(vertices), std::end(vertices));
	allIndices.reserve(allIndices.size() + indices.size());
	allIndices.insert(std::end(allIndices), std::begin(indices), std::end(indices));

	Rendering::OpenGLBackEnd::UpdateVertexBufferData(allVertices.data(), allVertices.size());
	Rendering::OpenGLBackEnd::UpdateIndexBufferData(allIndices.data(), allIndices.size());

	g_nextVertexInsert += mesh.vertexCount;
	g_nextIndexInsert += mesh.indexCount;
	return meshes.size() - 1;
}

uint32_t Moon::AssetManagment::MeshManager::CreateMesh(const std::string& name, const std::vector<Rendering::Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex) {
	glm::vec3 aabbMin = vertices[0].position;
	glm::vec3 aabbMax = vertices[0].position;
	for (const Rendering::Vertex& v : vertices) {
		aabbMin = glm::min(aabbMin, v.position);
		aabbMax = glm::max(aabbMax, v.position);
	}

	return CreateMesh(name, vertices, indices, materialIndex, aabbMin, aabbMax);
}

Moon::Mesh* Moon::AssetManagment::MeshManager::GetMeshByIndex(uint32_t index) {
	if (index >= 0 && index < m_meshes.size()) {
		return &m_meshes[index];
	}
	else {
		HZ_CORE_ERROR("MeshManager::GetMeshByIndex() failed because '{}' does not exists\n", index);
	}
	return nullptr;
}

Moon::Mesh* Moon::AssetManagment::MeshManager::GetMeshByName(const std::string& name) {
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i].name == name)
			return &m_meshes[i];
	}
	HZ_CORE_ERROR("MeshManager::GetMeshByName() failed because '" + name + "' does not exists\n");
	return nullptr;
}

uint32_t Moon::AssetManagment::MeshManager::GetMeshIndexByName(const std::string& name) {
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i].name == name)
			return i;
	}
	HZ_CORE_ERROR("MeshManager::GetMeshIndexByName() failed because '" + name + "' does not exists\n");
	return 0;
}

Moon::Mesh* Moon::AssetManagment::MeshManager::GetQuad() {
	return GetMeshByName("Quad");
}

uint32_t Moon::AssetManagment::MeshManager::CreateSkinnedMesh(const std::string& name, const std::vector<Rendering::WeightedVertex>& vertices, const std::vector<uint32_t>& indices, uint32_t baseVertexLocal, glm::vec3 aabbMin, glm::vec3 aabbMax, uint32_t materialIndex) {
	std::vector<Rendering::SkinnedMesh>& meshes = m_skinnedMeshes;
	std::vector<Rendering::WeightedVertex>& allVertices = m_skinnedVertices;
	std::vector<uint32_t>& allIndices = m_skinnedIndices;

	Rendering::SkinnedMesh& mesh = meshes.emplace_back();
	mesh.baseVertexGlobal = static_cast<uint32_t>(m_vertices.size());
	mesh.baseVertexLocal = baseVertexLocal;
	mesh.baseIndex = static_cast<uint32_t>(m_indices.size());
	mesh.vertexCount = static_cast<uint32_t>(vertices.size());
	mesh.indexCount = static_cast<uint32_t>(indices.size());
	mesh.materialIndex = materialIndex;
	mesh.name = name;
	mesh.aabbMin = aabbMin;
	mesh.aabbMax = aabbMax;

	//HZ_CORE_TRACE("[DEBUG] Creating mesh: {}\n | Vertices: {}, Base: {}\n | Indices: {}, Base: {}",
	//	name, vertices.size(), mesh.baseVertex, indices.size(), mesh.firstIndex);

	allVertices.reserve(allVertices.size() + vertices.size());
	allVertices.insert(std::end(allVertices), std::begin(vertices), std::end(vertices));
	allIndices.reserve(allIndices.size() + indices.size());
	allIndices.insert(std::end(allIndices), std::begin(indices), std::end(indices));

	Rendering::OpenGLBackEnd::UpdateWightedVertexData(allVertices.data(), allVertices.size());
	Rendering::OpenGLBackEnd::UpdateWightedIndexData(allIndices.data(), allIndices.size());

	g_nextWeightedVertexInsert += mesh.vertexCount;
	g_nextWeightedIndexInsert += mesh.indexCount;
	return meshes.size() - 1;
}

std::vector<Moon::Rendering::Vertex>& Moon::AssetManagment::MeshManager::GetVertices() {
	return m_vertices;
}

std::vector<uint32_t>& Moon::AssetManagment::MeshManager::GetIndices() {
	return m_indices;
}
