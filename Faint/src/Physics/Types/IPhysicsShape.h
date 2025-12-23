#pragma once

#include <glm/glm.hpp>

namespace Moon::Physics {
	struct ShapeInfo {
		glm::vec3 halfSize; // For box
		float radius;		// For sphere
		float height;		// For capsule
	};

	class IPhysicsShape {
	public:
		virtual void CreateShape(const ShapeInfo& p_info) = 0;
		virtual void RecreateShape(const ShapeInfo& p_info) = 0;
	};
}