#pragma once
#include "Vertex.h"

namespace Moon::Rendering {
	struct WeightedVertex {
		glm::vec3 position = glm::vec3(0);
		glm::vec3 normal = glm::vec3(0);
		glm::vec2 uv = glm::vec2(0);
		glm::vec3 tangent = glm::vec3(0);
		glm::ivec4 boneID = glm::ivec4(0);
		glm::vec4 weight = glm::vec4(0);
		bool operator==(const Vertex& other) const {
			return position == other.position && normal == other.normal && uv == other.uv;
		}
		bool operator==(const WeightedVertex& other) const {
			return position == other.position && normal == other.normal && uv == other.uv;
		}
	};
}