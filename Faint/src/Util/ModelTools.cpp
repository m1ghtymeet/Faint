#include "ModelTools.h"
#include <fstream>

void Moon::ModelTools::ExportModel(const ModelData& model, const std::string& filepath) {

	std::ofstream out(filepath, std::ios::binary);
	if (!out.is_open()) return;

	// --- Model name ---
	uint32_t nameLength = (uint32_t)model.name.size();
	out.write(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
	out.write(model.name.data(), nameLength);

	// --- Model metadata ---
	out.write((char*)(&model.meshCount), sizeof(uint32_t));
	out.write((char*)(&model.timestamp), sizeof(uint64_t));
	out.write(reinterpret_cast<const char*>(&model.aabbMin), sizeof(glm::vec3));
	out.write(reinterpret_cast<const char*>(&model.aabbMax), sizeof(glm::vec3));

	// --- Materials ---
	uint32_t materialCount = static_cast<uint32_t>(model.materialNames.size());
	out.write(reinterpret_cast<char*>(&materialCount), sizeof(uint32_t));
	for (const auto& matName : model.materialNames) {
		uint32_t len = static_cast<uint32_t>(matName.size());
		out.write(reinterpret_cast<char*>(&len), sizeof(uint32_t));
		out.write(matName.data(), len);
	}

	// --- Meshes ---
	for (const auto& mesh : model.meshes) {
		uint32_t meshNameLen = (uint32_t)mesh.name.size();
		out.write(reinterpret_cast<char*>(&meshNameLen), sizeof(uint32_t));
		out.write(mesh.name.data(), meshNameLen);

		out.write(reinterpret_cast<const char*>(&mesh.vertexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&mesh.indexCount), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&mesh.parentIndex), sizeof(uint32_t));
		out.write(reinterpret_cast<const char*>(&mesh.aabbMin), sizeof(glm::vec3));
		out.write(reinterpret_cast<const char*>(&mesh.aabbMax), sizeof(glm::vec3));

		// Material Index
		out.write(reinterpret_cast<const char*>(&mesh.materialIndex), sizeof(uint32_t));

		//out.write((char*)&mesh.localTransform, sizeof(glm::mat4));
		//out.write((char*)&mesh.inverseBindTransform, sizeof(glm::mat4));

		// Vertices & Indices
		out.write(reinterpret_cast<const char*>(mesh.vertices.data()), sizeof(Rendering::Vertex) * mesh.vertices.size());
		out.write(reinterpret_cast<const char*>(mesh.indices.data()), sizeof(uint32_t) * mesh.indices.size());
	}

	out.close();
}

Moon::ModelData Moon::ModelTools::ImportModel(const std::string& filepath) {
	ModelData model;

	std::ifstream in(filepath, std::ios::binary);
	if (!in.is_open()) return model;

	// --- Model name ---
	uint32_t nameLength;
	in.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
	model.name.resize(nameLength);
	in.read(model.name.data(), nameLength);

	// --- Model metadata ---
	in.read(reinterpret_cast<char*>(&model.meshCount), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&model.timestamp), sizeof(uint64_t));
	in.read(reinterpret_cast<char*>(&model.aabbMin), sizeof(glm::vec3));
	in.read(reinterpret_cast<char*>(&model.aabbMax), sizeof(glm::vec3));

	// --- Materials ---
	uint32_t materialCount;
	in.read(reinterpret_cast<char*>(&materialCount), sizeof(uint32_t));
	model.materialNames.resize(materialCount);
	for (uint32_t i = 0; i < materialCount; i++) {
		uint32_t len;
		in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
		std::vector<char> buffer(len);
		in.read(buffer.data(), len);
		model.materialNames[i] = std::string(buffer.begin(), buffer.end());
	}

	// --- Meshes ---
	model.meshes.resize(model.meshCount);
	for (auto& mesh : model.meshes) {
		uint32_t meshNameLen;
		in.read(reinterpret_cast<char*>(&meshNameLen), sizeof(uint32_t));
		std::vector<char> meshNameBuf(meshNameLen);
		in.read(meshNameBuf.data(), meshNameLen);
		mesh.name = std::string(meshNameBuf.begin(), meshNameBuf.end());

		in.read(reinterpret_cast<char*>(&mesh.vertexCount), sizeof(uint32_t));
		in.read(reinterpret_cast<char*>(&mesh.indexCount), sizeof(uint32_t));
		in.read(reinterpret_cast<char*>(&mesh.parentIndex), sizeof(uint32_t));
		in.read(reinterpret_cast<char*>(&mesh.aabbMin), sizeof(glm::vec3));
		in.read(reinterpret_cast<char*>(&mesh.aabbMax), sizeof(glm::vec3));

		in.read(reinterpret_cast<char*>(&mesh.materialIndex), sizeof(uint32_t));

		// Vertices
		mesh.vertices.resize(mesh.vertexCount);
		in.read(reinterpret_cast<char*>(mesh.vertices.data()), sizeof(Rendering::Vertex) * mesh.vertexCount);

		// Indices
		mesh.indices.resize(mesh.indexCount);
		in.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint32_t) * mesh.indexCount);
	}

	in.close();
	return model;
}
