#include "Ray.h"

Ray Moon::Math::GetMouseRay(int mouseX, int mouseY, float viewportWidth, float viewportHeight, const ICamera& camera) {
    glm::vec2 ndc = {
        (2.0f * mouseX) / viewportWidth - 1.0f,
        1.0f - (2.0f * mouseY) / viewportHeight
    };
    glm::vec4 clip(ndc.x, ndc.y, -1.0f, 1.0f);
    glm::mat4 invProj = glm::inverse(camera.GetProjectionMatrix());
    glm::mat4 invView = glm::inverse(camera.GetViewMatrix());
    glm::vec4 eye = invProj * clip;
    eye.z = -1.0f;
    eye.w = 0.0f;
    glm::vec3 worldDir = glm::normalize(glm::vec3(invView * eye));
    glm::vec3 worldOrigin = glm::vec3(invView[3]);
    return { worldOrigin, worldDir };
}
