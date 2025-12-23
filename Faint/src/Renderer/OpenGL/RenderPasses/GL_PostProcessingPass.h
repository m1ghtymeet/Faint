#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Renderer/Shader.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>

namespace Moon::Rendering {

    struct BloomSettings {
        float threshold = 1.0f;           // نقطه شروع درخشش
        float softKnee = 0.5f;            // نرمی گذار threshold
        float intensity = 0.8f;           // قدرت نهایی Bloom
        glm::vec3 tint = glm::vec3(1.0f); // رنگ Bloom
        int downsampleSteps = 5;          // تعداد سطوح MIP (بیشتر = نرم‌تر)
        float scatter = 0.7f;             // پخش نور
        bool useDirtMask = false;         // استفاده از Dirt Mask
        float dirtIntensity = 1.0f;       // قدرت Dirt Mask
    };

    struct GodRaySettings {
        float density = 0.8f;          // چگالی ray ها
        float decay = 0.95f;           // محو شدن
        float exposure = 0.3f;         // روشنایی
        float weight = 0.5f;           // وزن هر sample
        int numSamples = 32;           // تعداد samples (32-64)
        bool useTemporal = true;       // Temporal smoothing
        glm::vec3 lightPosition = glm::vec3(0); // Screen-space light pos
        float lightIntensity = 1.0f;
    };

	class BloomRenderPass : public ARenderPass {
	public:
		explicit BloomRenderPass(CompositeRenderer& p_renderer);
        ~BloomRenderPass() override;

		virtual void Draw() override;
		void ApplyBloom(GLuint sourceTexture, GLuint targetFBO);

		GLuint GetBloomTexture() const { return m_bloomResult; }

        void SetSettings(const BloomSettings& settings) { m_settings = settings; }
        BloomSettings& GetSettings() { return m_settings; }

        // Dirt Mask
        void SetDirtMask(GLuint texture) { m_dirtMaskTexture = texture; }

	private:
        void InitializeFramebuffers();
        void CleanupFramebuffers();
        void ResizeIfNeeded(int width, int height);

        // Bloom Stages
        void PrefilterPass(GLuint sourceTexture);
        void DownsamplePass();
        void UpsamplePass();
        void CompositePass(GLuint targetFBO);

        // Helper
        void DrawQuad();

        struct MipLevel {
            FrameBuffer* framebuffer = nullptr;
            int width = 0;
            int height = 0;
        };

        std::vector<MipLevel> m_mipChain;
        FrameBuffer* m_prefilterFBO = nullptr;

        BloomSettings m_settings;
        GLuint m_bloomResult = 0;
        GLuint m_dirtMaskTexture = 0;

        // Shaders
        std::shared_ptr<Shader> m_prefilterShader;
        std::shared_ptr<Shader> m_downsampleShader;
        std::shared_ptr<Shader> m_upsampleShader;
        std::shared_ptr<Shader> m_compositeShader;


        int m_currentWidth = 0;
        int m_currentHeight = 0;

        // Performance Stats
        struct {
            float prefilterTime = 0.0f;
            float downsampleTime = 0.0f;
            float upsampleTime = 0.0f;
            float totalTime = 0.0f;
        } m_stats;
	};

    class GodRayRenderPass : public ARenderPass {
    public:
        explicit GodRayRenderPass(CompositeRenderer& p_renderer);
        ~GodRayRenderPass() override;

        virtual void Draw() override {};

        void ApplyGodRays(GLuint sceneTexture, GLuint depthTexture, GLuint velocityTexture, GLuint targetFBO);
        GLuint GetGodRayTexture() const { return m_godRayResult; }
        void SetSettings(const GodRaySettings& settings) { m_settings = settings; }
        GodRaySettings& GetSettings() { return m_settings; }
        void UpdateLightPosition(const glm::vec2& screenPos) { m_settings.lightPosition = glm::vec3(screenPos, 0.0f); }

    private:
        void InitializeFramebuffers();
        void CleanupFramebuffers();
        void ResizeIfNeeded(int width, int height);

        // Pipeline stages
        void DetectLightSource(GLuint sceneTexture, GLuint depthTexture);
        void OcclusionPass(GLuint depthTexture);
        void RadialBlurPass(GLuint occlusionTexture);
        void TemporalAccumulation(GLuint velocityTexture);
        void CompositePass(GLuint targetFBO);

        void DrawQuad();

        struct MipLevel {
            FrameBuffer* framebuffer = nullptr;
            int width = 0, height = 0;
        };
        std::vector<MipLevel> m_blurMips;
        FrameBuffer* m_occlusionFBO = nullptr;
        FrameBuffer* m_accumulationFBO = nullptr;

        GodRaySettings m_settings;
        GLuint m_godRayResult = 0;
        GLuint m_lightSourceTex = 0;
        GLuint m_occlusionTex = 0;
        GLuint m_historyTex = 0;  // برای temporal
        glm::mat4 m_prevViewProj;

        // Shaders
        std::shared_ptr<Shader> m_detectShader;
        std::shared_ptr<Shader> m_occlusionShader;
        std::shared_ptr<Shader> m_radialBlurShader;
        std::shared_ptr<Shader> m_temporalShader;
        std::shared_ptr<Shader> m_compositeShader;

        int m_currentWidth = 0, m_currentHeight = 0;
    };

	class PostProcessingPass : public ARenderPass {
	public:
		PostProcessingPass(CompositeRenderer& p_renderer);
        ~PostProcessingPass() override;

		virtual void Draw() override;
		void BlitBackBuffer(int width, int height);
		int GetID();

	private:
		void SSAOPass();

		FrameBuffer* m_framebuffer = nullptr;
	};
}