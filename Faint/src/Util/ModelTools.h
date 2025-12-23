#pragma once

#include "AssimpImporter.h"
#include <Math/AABB.h>
#include <string>
#include <vector>

namespace Moon::ModelTools {

	struct ModelHeader {
		uint32_t version;
		uint32_t meshCount;
		uint32_t materialCount;
	};

	struct MeshHeader {
		char name[256];
		uint32_t vertexCount;
		uint32_t indexCount;
		AABB bounds;
	};

	void ExportModel(const ModelData& model, const std::string& filepath);
	ModelData ImportModel(const std::string& filepath);
}