#pragma once
#include <glm/glm.hpp>

namespace Moon::Rendering {
	struct Vertex {
		glm::vec3 position = glm::vec3(0);
		glm::vec2 uv = glm::vec2(0);
		glm::vec3 normal = glm::vec3(0);
		glm::vec3 tangent = glm::vec3(0);
		bool operator==(const Vertex& other) const {
			return position == other.position && normal == other.normal && uv == other.uv;
		}
	};
}