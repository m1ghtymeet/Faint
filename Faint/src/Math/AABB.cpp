#include "AABB.h"
#include <algorithm>
#include <Math/Math.h>

namespace {
    float DistanceSquared(glm::vec3 A, glm::vec3 B) {
        glm::vec3 C = A - B;
        return glm::dot(C, C);
    }
}

Moon::AABB::AABB(glm::vec3 min, glm::vec3 max) {
	boundsMin = min;
	boundsMax = max;
	CalculateCenter();
}

void Moon::AABB::Grow(AABB& b) {
	if (b.boundsMin.x != 1e30f && b.boundsMin.x != -1e30f) {
		Grow(b.boundsMin); Grow(b.boundsMax);
	}
	AABB::CalculateCenter();
}

void Moon::AABB::Grow(glm::vec3 p) {
	boundsMin = glm::vec3(std::min(boundsMin.x, p.x), std::min(boundsMin.y, p.y), std::min(boundsMin.z, p.z));
	boundsMax = glm::vec3(std::min(boundsMax.x, p.x), std::min(boundsMax.y, p.y), std::min(boundsMax.z, p.z));
	CalculateCenter();
}

float Moon::AABB::Area() {
	glm::vec3 e = boundsMax - boundsMin; // Box extent
	return e.x * e.y + e.y * e.z + e.z * e.x;
}

const glm::vec3 Moon::AABB::GetCenter() const {
	return center;
}
const glm::vec3 Moon::AABB::GetBoundsMin() const {
	return boundsMin;
}
const glm::vec3 Moon::AABB::GetBoundsMax() const {
	return boundsMax;
}

void Moon::AABB::CalculateCenter() {
    center = { (boundsMin.x + boundsMax.x) / 2, (boundsMin.y + boundsMax.y) / 2, (boundsMin.z + boundsMax.z) / 2 };
}

bool Moon::AABB::ContainsPoint(glm::vec3 point) const {
    return (point.x >= boundsMin.x && point.x <= boundsMax.x) &&
        (point.y >= boundsMin.y && point.y <= boundsMax.y) &&
        (point.z >= boundsMin.z && point.z <= boundsMax.z);
}

bool Moon::AABB::IntersectsSphere(const glm::vec3& sphereCenter, float radius) const {
    glm::vec3 closestPoint = glm::clamp(sphereCenter, boundsMin, boundsMax);
    float distSq = DistanceSquared(closestPoint, sphereCenter);
    return distSq <= (radius * radius);
}

bool Moon::AABB::IntersectsAABB(const AABB& other) const {
    return (boundsMin.x <= other.boundsMax.x && boundsMax.x >= other.boundsMin.x) &&
        (boundsMin.y <= other.boundsMax.y && boundsMax.y >= other.boundsMin.y) &&
        (boundsMin.z <= other.boundsMax.z && boundsMax.z >= other.boundsMin.z);
}

bool Moon::AABB::IntersectsAABB(const Moon::AABB& other, float threshold) const {
    glm::vec3 inflatedMinA = boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxA = boundsMax + glm::vec3(threshold);
    glm::vec3 inflatedMinB = other.boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxB = other.boundsMax + glm::vec3(threshold);

    return (inflatedMinA.x <= inflatedMaxB.x && inflatedMaxA.x >= inflatedMinB.x) &&
        (inflatedMinA.y <= inflatedMaxB.y && inflatedMaxA.y >= inflatedMinB.y) &&
        (inflatedMinA.z <= inflatedMaxB.z && inflatedMaxA.z >= inflatedMinB.z);
}

bool Moon::AABB::RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
    float tmin = (boundsMin.x - rayOrigin.x) / rayDirection.x;
    float tmax = (boundsMax.x - rayOrigin.x) / rayDirection.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (boundsMin.y - rayOrigin.y) / rayDirection.y;
    float tymax = (boundsMax.y - rayOrigin.y) / rayDirection.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = std::max(tmin, tymin);
    tmax = std::min(tmax, tymax);

    float tzmin = (boundsMin.z - rayOrigin.z) / rayDirection.z;
    float tzmax = (boundsMax.z - rayOrigin.z) / rayDirection.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    return !((tmin > tzmax) || (tzmin > tmax));
}
