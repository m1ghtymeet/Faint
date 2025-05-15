#include "File.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <chrono>

/*
█▄ ▄█ █▀█ █▀▄ █▀▀ █   █▀▀
█ █ █ █ █ █ █ █▀▀ █   ▀▀█
▀   ▀ ▀▀▀ ▀▀  ▀▀▀ ▀▀▀ ▀▀▀ */

void Faint::File::ExportModel(const ModelData& modelData) {

	std::string outputPath = modelData.name;
	std::ofstream file(outputPath, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Failed to open file writing: " << outputPath << "\n";
		return;
	}

	ModelHeader modelHeader;
	modelHeader.version = 1;
	modelHeader.meshCount = modelData.meshCount;
	modelHeader.nameLength = modelData.name.size();
	modelHeader.timestamp = modelData.timestamp;
	modelHeader.aabbMin = modelData.aabbMin;
	modelHeader.aabbMax = modelData.aabbMax;
	std::memcpy(modelHeader.signature, "FAINT_MODEL", 11);

	file.write(modelHeader.signature, 11);
	file.write((char*)&modelHeader.version, sizeof(modelHeader.version));
	file.write((char*)&modelHeader.meshCount, sizeof(modelHeader.meshCount));
	file.write((char*)&modelHeader.nameLength, sizeof(modelHeader.nameLength));
	file.write(reinterpret_cast<char*>(&modelHeader.timestamp), sizeof(modelHeader.timestamp));
	file.write(reinterpret_cast<const char*>(&modelHeader.aabbMin), sizeof(glm::vec3));
	file.write(reinterpret_cast<const char*>(&modelHeader.aabbMax), sizeof(glm::vec3));

	file.write(modelData.name.data(), modelHeader.nameLength);

	// Write the mesh data
	for (const MeshData& meshData : modelData.meshes) {
		MeshHeader meshHeader;
		meshHeader.nameLength = (uint32_t)meshData.name.size();
		meshHeader.vertexCount = (uint32_t)meshData.vertices.size();
		meshHeader.indexCount = (uint32_t)meshData.indices.size();
		meshHeader.aabbMin = meshData.aabbMin;
		meshHeader.aabbMax = meshData.aabbMax;
		file.write((char*)&meshHeader.nameLength, sizeof(meshHeader.nameLength));
		file.write((char*)&meshHeader.vertexCount, sizeof(meshHeader.vertexCount));
		file.write((char*)&meshHeader.indexCount, sizeof(meshHeader.indexCount));
		file.write(meshData.name.data(), meshHeader.nameLength);
		file.write(reinterpret_cast<const char*>(&meshHeader.aabbMin), sizeof(glm::vec3));
		file.write(reinterpret_cast<const char*>(&meshHeader.aabbMax), sizeof(glm::vec3));
		file.write(reinterpret_cast<const char*>(meshData.vertices.data()), meshData.vertices.size() * sizeof(Vertex));
		file.write(reinterpret_cast<const char*>(meshData.indices.data()), meshData.indices.size() * sizeof(uint32_t));
	}
	file.close();
	std::cout << "Exported: " << outputPath << "\n";
}