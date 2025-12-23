#pragma once
#include <glm/glm.hpp>

namespace Moon::Rendering {
	struct Vertex2D {
		glm::vec2 position;
		glm::vec2 uv;
		glm::vec4 color;
	};
}