#include "GLFTImporter.h"
#include <Debug/Log.h>
#define TINYGLTF_IMPLEMENTATION
#include "vendor/tinygltf/tiny_gltf.h"

Moon::ModelData Moon::GLTFImporter::ImportGltf(const std::string& filepath) {
    ModelData data;

	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	bool loaded = loader.LoadASCIIFromFile(&model, nullptr, nullptr, filepath);
	if (!loaded) {
		HZ_CORE_ERROR("Failed to load model: {}", filepath);
		return data;
	}
	data.meshes.clear();

	// Process meshes
	for (const auto& mesh : model.meshes) {
		MeshData meshData;

		for (const auto& primitive : mesh.primitives) {
			Rendering::Vertex vertex;
			if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				const float* positions = reinterpret_cast<const float*>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
				for (size_t i = 0; i < accessor.count; ++i) {
					glm::vec3 position;
					position.x = positions[i * 3 + 0];
					position.y = positions[i * 3 + 1];
					position.z = positions[i * 3 + 2];
					vertex.position = position;
				}
			}

			if (primitive.indices >= 0) {
				const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				const unsigned short* indices = reinterpret_cast<const unsigned short*>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
				for (size_t i = 0; i < accessor.count; ++i) {
					meshData.indices.push_back(indices[i]);
				}
			}
			meshData.vertices.push_back(vertex);
		}

		data.meshes.push_back(meshData);
	}

	HZ_CORE_TRACE("Meshes: {}", data.meshes.size());
    return data;
}
