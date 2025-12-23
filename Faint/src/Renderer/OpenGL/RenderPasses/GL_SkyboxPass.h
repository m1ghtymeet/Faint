#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>

namespace Moon::Rendering {

    struct SkySettings {
        glm::vec3 sunDirection = glm::vec3(0.0f, 0.5f, 0.8f);
        float sunIntensity = 20.0f;
        float sunSize = 0.015f;
        glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f);

        float atmosphereThickness = 1.0f;
        float rayleighScatter = 1.0f;
        float mieScatter = 0.8f;
        float exposure = 1.0f;

        float timeOfDay = 12.0f; // 0-24 hours

        bool enableClouds = false;
        float cloudSpeed = 0.1f;
        float cloudDensity = 0.5f;
    };

    class SkyboxRenderPass : public ARenderPass {
    public:
        explicit SkyboxRenderPass(CompositeRenderer& p_renderer);
        ~SkyboxRenderPass() override;

        void Draw() override;

        void SetTimeOfDay(float hours);

        void UpdateSunDirection(float azimuth, float elevation);

    private:
        void RenderSkybox();

        SkySettings m_settings;
        float m_time = 0.0f;
    };
}