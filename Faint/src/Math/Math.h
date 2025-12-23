#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include "glm/gtx/matrix_decompose.hpp"
#include <glm/gtx/quaternion.hpp>

namespace Moon::Math {
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::quat& outRotation, glm::vec3& outScale);

    inline glm::vec3 QuaternionToEulerDegrees(const glm::quat& q) {
        glm::quat n = glm::normalize(q);
        glm::mat3 rotMat = glm::mat3_cast(n);
        glm::vec3 euler = glm::eulerAngles(n);
        euler = glm::degrees(euler);
        for (int i = 0; i < 3; ++i) {
            if (euler[i] > 180.0f) euler[i] -= 360.0f;
            if (euler[i] < -180.0f) euler[i] += 360.0f;
        }
        return euler;
    }
    inline glm::quat EulerDegreesToQuatSafe(const glm::vec3& eulerDeg) {
        glm::vec3 clamped = eulerDeg;
        for (int i = 0; i < 3; ++i) {
            while (clamped[i] > 180.0f)  clamped[i] -= 360.0f;
            while (clamped[i] < -180.0f) clamped[i] += 360.0f;
        }
        return glm::quat(glm::radians(clamped));
    }
}