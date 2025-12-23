#pragma once
#include "Math/AABB.h"

#include <Renderer/Types/Vertex.h>
#include <Renderer/Types/IMesh.h>
#include <string>
#include <vector>

namespace Moon {
	struct Mesh {
		uint32_t firstIndex  = 0;
		uint32_t  baseVertex  = 0;
		uint32_t indexCount  = 0;
		uint32_t vertexCount = 0;

		uint32_t materialIndex = 0;
		uint32_t meshletOffset = 0;
		uint32_t meshletCount  = 0;

		glm::vec3 aabbMin, aabbMax;
		std::string name;
	};
}