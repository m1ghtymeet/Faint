#pragma once
#include <glm/glm.hpp>

namespace Moon {
	class Entity;
}

struct ColliderGizmoState {
	Moon::Entity* target = nullptr;
	int hoveredHandle = -1;
	int selectedHandle = -1;
	glm::vec3 handlePositions[8];
	glm::vec3 centerHandle;
};