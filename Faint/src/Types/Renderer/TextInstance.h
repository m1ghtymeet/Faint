#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Moon::Rendering {
	struct TextVertex {
		glm::vec2 position;
		glm::vec2 uv;
	};

	struct TextInstance {
		glm::vec2 position;
		glm::vec2 scale;
		glm::vec3 color;
		glm::vec4 atlasCoords;
	};
}