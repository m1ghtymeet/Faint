#pragma once
#include <glm/glm.hpp>
#include <Core/Rendering/ICamera.h>

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

struct RayHit {
	float distance;
	glm::vec3 point;
	int handleIndex;
};

namespace Moon::Math {
    Ray GetMouseRay(int mouseX, int mouseY, float viewportWidth, float viewportHeight, const ICamera& camera);
    inline bool RaySphereIntersection(const Ray& ray, const glm::vec3& sphereCenter, float sphereRadius, float& t) {
        glm::vec3 oc = ray.origin - sphereCenter;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) return false;
        t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
        return t >= 0;
    }
}
