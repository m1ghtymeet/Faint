#include "AABB.h"
#include "Util/Util.h"
#include <algorithm>

Faint::AABB::AABB(glm::vec3 min, glm::vec3 max) {
	boundsMin = min;
	boundsMax = max;
	CalculateCenter();
}

void Faint::AABB::Grow(AABB& b) {
	if (b.boundsMin.x != 1e30f && b.boundsMin.x != -1e30f) {
		Grow(b.boundsMin); Grow(b.boundsMax);
	}
	AABB::CalculateCenter();
}

void Faint::AABB::Grow(glm::vec3 p) {
	boundsMin = glm::vec3(std::min(boundsMin.x, p.x), std::min(boundsMin.y, p.y), std::min(boundsMin.z, p.z));
	boundsMax = glm::vec3(std::min(boundsMax.x, p.x), std::min(boundsMax.y, p.y), std::min(boundsMax.z, p.z));
	CalculateCenter();
}

float Faint::AABB::Area() {
	glm::vec3 e = boundsMax - boundsMin; // Box extent
	return e.x * e.y + e.y * e.z + e.z * e.x;
}

const glm::vec3 Faint::AABB::GetCenter() const {
	return center;
}
const glm::vec3 Faint::AABB::GetBoundsMin() const {
	return boundsMin;
}
const glm::vec3 Faint::AABB::GetBoundsMax() const {
	return boundsMax;
}

void Faint::AABB::CalculateCenter() {
    center = { (boundsMin.x + boundsMax.x) / 2, (boundsMin.y + boundsMax.y) / 2, (boundsMin.z + boundsMax.z) / 2 };
}

bool Faint::AABB::ContainsPoint(glm::vec3 point) const {
    return (point.x >= boundsMin.x && point.x <= boundsMax.x) &&
        (point.y >= boundsMin.y && point.y <= boundsMax.y) &&
        (point.z >= boundsMin.z && point.z <= boundsMax.z);
}

bool Faint::AABB::IntersectsSphere(const glm::vec3& sphereCenter, float radius) const {
    glm::vec3 closestPoint = glm::clamp(sphereCenter, boundsMin, boundsMax);
    float distSq = Util::DistanceSquared(closestPoint, sphereCenter);
    return distSq <= (radius * radius);
}

bool Faint::AABB::IntersectsAABB(const AABB& other) const {
    return (boundsMin.x <= other.boundsMax.x && boundsMax.x >= other.boundsMin.x) &&
        (boundsMin.y <= other.boundsMax.y && boundsMax.y >= other.boundsMin.y) &&
        (boundsMin.z <= other.boundsMax.z && boundsMax.z >= other.boundsMin.z);
}

bool Faint::AABB::IntersectsAABB(const Faint::AABB& other, float threshold) const {
    glm::vec3 inflatedMinA = boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxA = boundsMax + glm::vec3(threshold);
    glm::vec3 inflatedMinB = other.boundsMin - glm::vec3(threshold);
    glm::vec3 inflatedMaxB = other.boundsMax + glm::vec3(threshold);

    return (inflatedMinA.x <= inflatedMaxB.x && inflatedMaxA.x >= inflatedMinB.x) &&
        (inflatedMinA.y <= inflatedMaxB.y && inflatedMaxA.y >= inflatedMinB.y) &&
        (inflatedMinA.z <= inflatedMaxB.z && inflatedMaxA.z >= inflatedMinB.z);
}

bool Faint::AABB::RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
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
