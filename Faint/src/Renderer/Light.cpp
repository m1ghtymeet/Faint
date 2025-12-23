#include "Light.h"
#include <Math/Math.h>
#include <iostream>

glm::mat4 Moon::Rendering::Light::CalculateMatrix() const {
    // Shadow matrix calculation can stay the same
    return glm::mat4(1);
}

glm::vec3 Moon::Rendering::Light::GetDirectionFromQuat(glm::quat q) const {
    return glm::normalize(q * glm::vec3(0, 0, -1));
}

Moon::Rendering::LightData Moon::Rendering::Light::PackLightData(LightType type, const glm::vec3& worldPosition, const glm::quat& worldRotation) const {
    LightData data;
    data.type = static_cast<int>(type);
    data.intensity = strength;
    data.colorR = color.r;
    data.colorG = color.g;
    data.colorB = color.b;
    data.padding[0] = 0.0f;
    data.padding[1] = 0.0f;
    data.padding[2] = 0.0f;

    data.data = glm::mat4(0.0f);

    switch (type) {
    case LightType::Point:
        data.data[0] = glm::vec4(worldPosition, constant);
        data.data[1] = glm::vec4(linear, quadratic, 0.0f, 0.0f);
        break;
    case LightType::Directional:
        // Pack directional light data into mat4
        glm::vec3 dir = glm::vec3(worldRotation.x, worldRotation.y, worldRotation.z);
        data.data[0] = glm::vec4(GetDirectionFromQuat(worldRotation) * glm::vec3(-1), castShadows ? 1.0f : 0.0f);
        break;
    case LightType::Spot: {
        // Pack spot light data into mat4
        data.data[0] = glm::vec4(worldPosition, constant);
        glm::vec3 dir = GetDirectionFromQuat(worldRotation);
        data.data[1] = glm::vec4(dir, 0.0f);
        data.data[2] = glm::vec4(linear, quadratic, cutoff, outerCutoff);
        break;
        }
    }

    return data;
}
