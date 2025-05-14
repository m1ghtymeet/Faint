#pragma once
#include <glm/vec3.hpp>

namespace Faint {
	struct AABB {
		AABB() = default;
		AABB(glm::vec3 min, glm::vec3 max);
		void Grow(AABB& b);
		void Grow(glm::vec3 p);
		float Area();
		const glm::vec3 GetCenter() const;
		const glm::vec3 GetBoundsMin() const;
		const glm::vec3 GetBoundsMax() const;
		bool IntersectsSphere(const glm::vec3& sphereCenter, float radius) const;
		bool IntersectsAABB(const AABB& other) const;
		bool IntersectsAABB(const AABB& other, float threshold) const;
		bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const;
		bool ContainsPoint(glm::vec3 point) const;

		glm::vec3 center = glm::vec3(0);
		glm::vec3 boundsMin = glm::vec3(1e30f);
		glm::vec3 boundsMax = glm::vec3(-1e30f);
		glm::vec3 padding = glm::vec3(0);

	private:
		void CalculateCenter();
	};
}