#include "Frustum.h"

void Moon::Math::Frustum::Update(const glm::mat4& viewProj) {
    const glm::mat4& m = viewProj;

    // Extract 6 planes (normalized)
    m_planes[0] = glm::normalize(m[3] + m[2]); // Near
    m_planes[1] = glm::normalize(m[3] - m[2]); // Far
    m_planes[2] = glm::normalize(m[3] + m[0]); // Left
    m_planes[3] = glm::normalize(m[3] - m[0]); // Right
    m_planes[4] = glm::normalize(m[3] - m[1]); // Bottom
    m_planes[5] = glm::normalize(m[3] + m[1]); // Top
}

bool Moon::Math::Frustum::IsAABBVisible(const AABB& aabb) const {
    const glm::vec3& min = aabb.GetBoundsMin();
    const glm::vec3& max = aabb.GetBoundsMax();

    for (const auto& plane : m_planes) {
        glm::vec3 positive = min;
        if (plane.x >= 0) positive.x = max.x;
        if (plane.y >= 0) positive.y = max.y;
        if (plane.z >= 0) positive.z = max.z;

        if (glm::dot(glm::vec3(plane), positive) + plane.w < 0)
            return false;
    }
    return true;
}
