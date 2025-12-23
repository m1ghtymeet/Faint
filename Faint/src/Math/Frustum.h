#pragma once
#include <glm/glm.hpp>
#include <array>
#include <Math/AABB.h>

namespace Moon::Math {
    enum class FrustumPlane : uint8_t {
        Near = 0,
        Far,
        Left,
        Right,
        Top,
        Bottom
    };

    class Frustum {
    public:
        void Update(const glm::mat4& viewProj);
        bool IsAABBVisible(const AABB& aabb) const;
        //bool IsSphereVisible(const glm::vec3& center, float radius) const;

    private:
        std::array<glm::vec4, 6> m_planes;
    };
}