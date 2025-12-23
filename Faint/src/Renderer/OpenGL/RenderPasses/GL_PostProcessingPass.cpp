#include "GL_PostProcessingPass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/Shader.h>
#include <Types/Renderer/Mesh.h>
#include <AssetManagment/MeshManager.h>

#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_UIPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_SkyboxPass.h>
#include <Renderer/PostProcess/SSAO.h>

namespace {
	Moon::Shader* shader = nullptr;
	GLuint m_noiseTexture = 0;
	GLuint g_finalSSAOTex = 0;
	//Moon::Mesh* g_quadMesh;
	Moon::FrameBuffer* g_ssaoBlurFBO = nullptr;
    //Moon::Rendering::BloomRenderPass* g_bloomPass = nullptr;
    //Moon::Rendering::GodRayRenderPass* g_godRayPass = nullptr;
}

Moon::Rendering::PostProcessingPass::PostProcessingPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer) {

	m_framebuffer = new FrameBuffer("PostProcessing", 1600, 900);
	m_framebuffer->CreateAttachment("Color", GL_RGB16F);
	m_framebuffer->CreateAttachment("SSAO", GL_RED);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	g_ssaoBlurFBO = new FrameBuffer("SSAOBlur", 1600, 900);
	g_ssaoBlurFBO->CreateAttachment("SSAO", GL_RGB16F);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = m_renderer.GetShader("postProcessing").get();
	
	//g_quadMesh = AssetManagment::MeshManager::GetQuad();

    //g_bloomPass = new BloomRenderPass(p_renderer);
    //g_godRayPass = new GodRayRenderPass(p_renderer);

    //auto& bloomSettings = g_bloomPass->GetSettings();
    //bloomSettings.threshold = 1.0f;      // Bright pixels با luminance > 1.0
    //bloomSettings.softKnee = 0.5f;       // نرمی transition
    //bloomSettings.intensity = 0.8f;      // قدرت نهایی
    //bloomSettings.scatter = 0.7f;        // پخش نور
    //bloomSettings.downsampleSteps = 5;   // 5 mip levels
    //bloomSettings.tint = glm::vec3(1.0f, 1.0f, 1.0f); // بدون tint
    //bloomSettings.useDirtMask = false;   // فعلاً خاموش
    //
    //auto& grSettings = g_godRayPass->GetSettings();
    //grSettings.density = 0.6f;
    //grSettings.decay = 0.92f;
    //grSettings.weight = 0.4f;
    //grSettings.numSamples = 24;
    //grSettings.exposure = 0.05f;

	PostProcess::SSAO::Precompute();

	glGenTextures(1, &m_noiseTexture);
	glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, PostProcess::SSAO::ssaoNoise.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

    GLuint kernelSSBO;
    glGenBuffers(1, &kernelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kernelSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * 64, &PostProcess::SSAO::ssaoKernel, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, kernelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

Moon::Rendering::PostProcessingPass::~PostProcessingPass() {
    //if (m_framebuffer) delete m_framebuffer;
    if (g_ssaoBlurFBO) delete g_ssaoBlurFBO;
    //if (g_bloomPass) delete g_bloomPass;
    if (m_noiseTexture) glDeleteTextures(1, &m_noiseTexture);
}

int Moon::Rendering::PostProcessingPass::GetID() {
	return m_framebuffer->GetColorAttachmentHandleByName("Color");
}

void Moon::Rendering::PostProcessingPass::SSAOPass() {

	Shader* shader = m_renderer.GetShader("ssao").get();
	Shader* ssaoBlurShader = m_renderer.GetShader("ssao_blur").get();
	auto gBuffer = m_renderer.GetFrameBuffer("GBuffer");

	m_framebuffer->Bind();
	m_framebuffer->DrawBuffer("SSAO");
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	shader->Bind();
	for (unsigned int i = 0; i < 64; ++i)
		shader->SetVec3("samples[" + std::to_string(i) + "]", PostProcess::SSAO::ssaoKernel[i]);
	shader->SetVec2("noiseSize", { m_renderer.GetFrameDescriptor().renderWidth, m_renderer.GetFrameDescriptor().renderHeight });

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer->GetColorAttachmentHandleByName("ScreenSpacePosition"));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer->GetColorAttachmentHandleByName("Normal"));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_noiseTexture);

    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

	shader->Unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bool horizontal = true;
	const int blurPasses = 2;

	FrameBuffer* src = m_framebuffer;
	FrameBuffer* dst = g_ssaoBlurFBO;

	for (int i = 0; i < blurPasses; i++) {
		dst->Bind();
		dst->DrawBuffer("SSAO");
		glClear(GL_COLOR_BUFFER_BIT);

		ssaoBlurShader->Bind();
		ssaoBlurShader->SetBool("horizontal", horizontal);
		ssaoBlurShader->SetInt("ssaoInput", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, src->GetColorAttachmentHandleByName("SSAO"));

        glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

		ssaoBlurShader->Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::swap(src, dst);
		horizontal = !horizontal;
	}

	g_finalSSAOTex = src->GetColorAttachmentHandleByName("SSAO");
}

void Moon::Rendering::PostProcessingPass::Draw() {

	int renderWidth = m_renderer.GetFrameDescriptor().renderWidth;
	int renderHeight = m_renderer.GetFrameDescriptor().renderHeight;
	m_framebuffer->Resize(renderWidth, renderHeight);
	g_ssaoBlurFBO->Resize(renderWidth, renderHeight);

	SSAOPass();

    GLuint sceneTextureID = m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("FinalLighting");
    //g_bloomPass->ApplyBloom(sceneTextureID, 0);
    //GLuint bloomTexID = g_bloomPass->GetBloomTexture();

    //GLuint depthTex = m_renderer.GetFrameBuffer("GBuffer")->GetDepthAttachmentHandle();
    //g_godRayPass->UpdateLightPosition(glm::vec2(1, 1));
    //g_godRayPass->ApplyGodRays(sceneTextureID, depthTex, 0, 0);
    //GLuint godRayTex = g_godRayPass->GetGodRayTexture();

	m_framebuffer->Bind();
	m_framebuffer->DrawBuffer("Color");
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

	shader->Bind();
	GLuint uiTextureID = m_renderer.GetPass<UIRenderPass>("UI").GetWidgetTextureID();
	GLuint uiTextTextureID = m_renderer.GetPass<UIRenderPass>("UI").GetText2DTextureID();
    GLuint depthTextureID = m_renderer.GetFrameBuffer("GBuffer")->GetDepthAttachmentHandle();

	glBindTextureUnit(0, sceneTextureID);
	glBindTextureUnit(1, uiTextureID);
	glBindTextureUnit(2, uiTextTextureID);
	glBindTextureUnit(3, g_finalSSAOTex);
	//glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, bloomTexID);
    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, depthTextureID);
    
    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::PostProcessingPass::BlitBackBuffer(int width, int height) {
	m_framebuffer->BlitToBackBuffer("Color", width, height);
}

Moon::Rendering::BloomRenderPass::BloomRenderPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer) {
	HZ_CORE_INFO("Initializing Professional Bloom Pass...");

	// Load shaders
	m_prefilterShader = m_renderer.GetShader("bloom_prefilter");
	m_downsampleShader = m_renderer.GetShader("bloom_downsample");
	m_upsampleShader = m_renderer.GetShader("bloom_upsample");
	m_compositeShader = m_renderer.GetShader("bloom_composite");

    //g_quadMesh = AssetManagment::MeshManager::GetQuad();

	InitializeFramebuffers();
}

Moon::Rendering::BloomRenderPass::~BloomRenderPass() {
    CleanupFramebuffers();
}

void Moon::Rendering::BloomRenderPass::InitializeFramebuffers() {
    // شروع با رزولوشن پیش‌فرض - بعداً Resize می‌شود
    int width = 1600;
    int height = 900;

    // Prefilter buffer
    m_prefilterFBO = new FrameBuffer("BloomPrefilter", width, height);
    m_prefilterFBO->CreateAttachment("Color", GL_RGB16F, GL_LINEAR, GL_LINEAR);

    // ایجاد Mip Chain
    m_mipChain.clear();
    int mipWidth = width; int mipHeight = height;
    for (int i = 0; i < m_settings.downsampleSteps; ++i) {
        MipLevel mip;
        mip.width = mipWidth;
        mip.height = mipHeight;

        std::string name = "BloomMip" + std::to_string(i);
        mip.framebuffer = new FrameBuffer(name.c_str(), mipWidth, mipHeight);
        mip.framebuffer->CreateAttachment("Color", GL_RGB16F, GL_LINEAR, GL_LINEAR);

        m_mipChain.push_back(mip);

        mipWidth = std::max(1, mipWidth / 2);
        mipHeight = std::max(1, mipHeight / 2);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::BloomRenderPass::CleanupFramebuffers() {
    if (m_prefilterFBO) {
        delete m_prefilterFBO;
        m_prefilterFBO = nullptr;
    }

    for (auto& mip : m_mipChain) {
        if (mip.framebuffer) {
            delete mip.framebuffer;
        }
    }
    m_mipChain.clear();
}

void Moon::Rendering::BloomRenderPass::ResizeIfNeeded(int width, int height) {
    if (m_currentWidth == width && m_currentHeight == height) {
        return;
    }

    m_currentWidth = width;
    m_currentHeight = height;

    // Resize prefilter
    m_prefilterFBO->Resize(width, height);

    // Resize mip chain
    int mipWidth = width / 2;
    int mipHeight = height / 2;

    for (int i = 0; i < m_settings.downsampleSteps && i < m_mipChain.size(); ++i) {
        m_mipChain[i].width = mipWidth;
        m_mipChain[i].height = mipHeight;
        m_mipChain[i].framebuffer->Resize(mipWidth, mipHeight);

        mipWidth = std::max(1, mipWidth / 2);
        mipHeight = std::max(1, mipHeight / 2);
    }
}

void Moon::Rendering::BloomRenderPass::Draw() {
    // این متد برای استفاده مستقل است
    // معمولاً ApplyBloom() از PostProcessing فراخوانی می‌شود
}

void Moon::Rendering::BloomRenderPass::ApplyBloom(GLuint sourceTexture, GLuint targetFBO) {
    auto startTime = std::chrono::high_resolution_clock::now();

    int width = m_renderer.GetFrameDescriptor().renderWidth;
    int height = m_renderer.GetFrameDescriptor().renderHeight;

    ResizeIfNeeded(width, height);

    // ذخیره state قبلی
    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // 1. Prefilter - استخراج bright pixels
    auto t1 = std::chrono::high_resolution_clock::now();
    PrefilterPass(sourceTexture);
    auto t2 = std::chrono::high_resolution_clock::now();
    m_stats.prefilterTime = std::chrono::duration<float, std::milli>(t2 - t1).count();

    // 2. Downsample - ایجاد Mip Chain
    auto t3 = std::chrono::high_resolution_clock::now();
    DownsamplePass();
    auto t4 = std::chrono::high_resolution_clock::now();
    m_stats.downsampleTime = std::chrono::duration<float, std::milli>(t4 - t3).count();

    // 3. Upsample - ترکیب و blur
    auto t5 = std::chrono::high_resolution_clock::now();
    UpsamplePass();
    auto t6 = std::chrono::high_resolution_clock::now();
    m_stats.upsampleTime = std::chrono::duration<float, std::milli>(t6 - t5).count();

    // نتیجه در m_mipChain[0] است
    m_bloomResult = m_mipChain[0].framebuffer->GetColorAttachmentHandleByName("Color");
    if (targetFBO != 0)
        CompositePass(targetFBO);

    // بازگردانی state
    if (depthTest) glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.totalTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    if (m_stats.totalTime > 5.0f) {
        HZ_CORE_WARN("Bloom time: {:.2f}ms (Prefilter: {:.2f}, Down: {:.2f}, Up: {:.2f})",
            m_stats.totalTime, m_stats.prefilterTime,
            m_stats.downsampleTime, m_stats.upsampleTime);
    }
}

void Moon::Rendering::BloomRenderPass::PrefilterPass(GLuint sourceTexture) {
    m_prefilterFBO->Bind();
    m_prefilterFBO->DrawBuffer("Color");
    glClear(GL_COLOR_BUFFER_BIT);

    m_prefilterShader->Bind();

    // Threshold parameters
    float knee = m_settings.threshold * m_settings.softKnee;
    glm::vec4 thresholdParams(
        m_settings.threshold,
        m_settings.threshold - knee,
        2.0f * knee,
        0.25f / (knee + 0.00001f)
    );

    m_prefilterShader->SetVec4("u_threshold", thresholdParams);
    m_prefilterShader->SetInt("u_sourceTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);

    DrawQuad();

    m_prefilterShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::BloomRenderPass::DownsamplePass() {
    m_downsampleShader->Bind();

    GLuint sourceTexture = m_prefilterFBO->GetColorAttachmentHandleByName("Color");

    for (int i = 0; i < m_settings.downsampleSteps && i < m_mipChain.size(); ++i) {
        auto& mip = m_mipChain[i];

        mip.framebuffer->Bind();
        mip.framebuffer->DrawBuffer("Color");
        glClear(GL_COLOR_BUFFER_BIT);

        // تنظیم viewport برای mip فعلی
        glViewport(0, 0, mip.width, mip.height);

        m_downsampleShader->SetInt("u_sourceTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);

        DrawQuad();

        // texture بعدی از mip فعلی می‌آید
        sourceTexture = mip.framebuffer->GetColorAttachmentHandleByName("Color");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_downsampleShader->Unbind();
}

void Moon::Rendering::BloomRenderPass::UpsamplePass() {
    m_upsampleShader->Bind();
    m_upsampleShader->SetFloat("u_scatter", m_settings.scatter);

    // شروع از کوچک‌ترین mip و رفتن به سمت بزرگ‌تر
    for (int i = m_settings.downsampleSteps - 1; i > 0; --i) {
        if (i >= m_mipChain.size()) continue;

        auto& currentMip = m_mipChain[i - 1];
        auto& higherMip = m_mipChain[i];

        currentMip.framebuffer->Bind();
        currentMip.framebuffer->DrawBuffer("Color");

        glViewport(0, 0, currentMip.width, currentMip.height);

        // Additive blending برای ترکیب mip levels
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        m_upsampleShader->SetInt("u_sourceTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, higherMip.framebuffer->GetColorAttachmentHandleByName("Color"));

        DrawQuad();

        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_upsampleShader->Unbind();

    // بازگردانی viewport به اندازه کامل
    int width = m_renderer.GetFrameDescriptor().renderWidth;
    int height = m_renderer.GetFrameDescriptor().renderHeight;
    glViewport(0, 0, width, height);
}

void Moon::Rendering::BloomRenderPass::CompositePass(GLuint targetFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    m_compositeShader->Bind();
    m_compositeShader->SetFloat("u_intensity", m_settings.intensity);
    m_compositeShader->SetVec3("u_tint", m_settings.tint);
    m_compositeShader->SetBool("u_useDirtMask", m_settings.useDirtMask);
    m_compositeShader->SetFloat("u_dirtIntensity", m_settings.dirtIntensity);

    m_compositeShader->SetInt("u_bloomTexture", 0);
    m_compositeShader->SetInt("u_dirtMask", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_bloomResult);

    if (m_settings.useDirtMask && m_dirtMaskTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_dirtMaskTexture);
    }

    DrawQuad();

    m_compositeShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::BloomRenderPass::DrawQuad() {
    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

Moon::Rendering::GodRayRenderPass::GodRayRenderPass(CompositeRenderer& p_renderer)
    : ARenderPass(p_renderer)
{
    //m_quadMesh = AssetManagment::MeshManager::GetQuad();

    // Load shaders
    m_detectShader = m_renderer.GetShader("godray_detect");
    m_occlusionShader = m_renderer.GetShader("godray_occlusion");
    m_radialBlurShader = m_renderer.GetShader("godray_radial_blur");
    m_temporalShader = m_renderer.GetShader("godray_temporal");
    m_compositeShader = m_renderer.GetShader("godray_composite");

    InitializeFramebuffers();
}

Moon::Rendering::GodRayRenderPass::~GodRayRenderPass() {
    CleanupFramebuffers();
}

void Moon::Rendering::GodRayRenderPass::InitializeFramebuffers() {
    int width = 1600, height = 900;

    m_occlusionFBO = new FrameBuffer("GodRayOcclusion", width / 4, height / 4);
    m_occlusionFBO->CreateAttachment("Occlusion", GL_R16F);

    // Accumulation buffer (quarter res)
    m_accumulationFBO = new FrameBuffer("GodRayAccum", width / 4, height / 4);
    m_accumulationFBO->CreateAttachment("Accum", GL_RGB16F);

    // Blur Mip Chain (5 levels)
    m_blurMips.clear();
    int mipW = width / 4, mipH = height / 4;
    for (int i = 0; i < 5; ++i) {
        MipLevel mip;
        mip.framebuffer = new FrameBuffer(("GodRayMip" + std::to_string(i)).c_str(), mipW, mipH);
        mip.framebuffer->CreateAttachment("Color", GL_RGB16F);
        mip.width = mipW; mip.height = mipH;
        m_blurMips.push_back(mip);
        mipW /= 2; mipH /= 2;
        if (mipW < 1) mipW = 1;
        if (mipH < 1) mipH = 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &m_historyTex);
    glBindTexture(GL_TEXTURE_2D, m_historyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width / 4, height / 4, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Moon::Rendering::GodRayRenderPass::CleanupFramebuffers() {
    if (m_occlusionFBO) delete m_occlusionFBO;
    if (m_accumulationFBO) delete m_accumulationFBO;
    for (auto& mip : m_blurMips) {
        if (mip.framebuffer) delete mip.framebuffer;
    }
    m_blurMips.clear();
    if (m_historyTex) glDeleteTextures(1, &m_historyTex);
    if (m_lightSourceTex) glDeleteTextures(1, &m_lightSourceTex);
}

void Moon::Rendering::GodRayRenderPass::ResizeIfNeeded(int width, int height) {
    if (m_currentWidth == width && m_currentHeight == height) return;

    m_currentWidth = width;
    m_currentHeight = height;

    int qWidth = width, qHeight = height;
    m_occlusionFBO->Resize(qWidth, qHeight);
    m_accumulationFBO->Resize(qWidth, qHeight);

    // Resize mips
    int mipW = qWidth, mipH = qHeight;
    for (size_t i = 0; i < m_blurMips.size(); ++i) {
        m_blurMips[i].framebuffer->Resize(mipW, mipH);
        m_blurMips[i].width = mipW;
        m_blurMips[i].height = mipH;
        mipW = std::max(1, mipW);
        mipH = std::max(1, mipH);
    }

    // Resize history
    glBindTexture(GL_TEXTURE_2D, m_historyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, qWidth, qHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Moon::Rendering::GodRayRenderPass::ApplyGodRays(GLuint sceneTexture, GLuint depthTexture, GLuint velocityTexture, GLuint targetFBO) {
    int width = m_renderer.GetFrameDescriptor().renderWidth;
    int height = m_renderer.GetFrameDescriptor().renderHeight;
    ResizeIfNeeded(width, height);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    //DetectLightSource(sceneTexture, depthTexture);

    OcclusionPass(depthTexture);
    RadialBlurPass(m_occlusionTex);

    // Stage 4: Temporal accumulation
    //if (m_settings.useTemporal) {
    //    TemporalAccumulation(velocityTexture);
    //}
    //else {
        m_godRayResult = m_blurMips[0].framebuffer->GetColorAttachmentHandleByName("Color");
    //}

    if (targetFBO != 0) {
        CompositePass(targetFBO);
    }
}

void Moon::Rendering::GodRayRenderPass::DetectLightSource(GLuint sceneTexture, GLuint depthTexture) {
    // ساده: از lightPosition دستی استفاده کن
    // TODO: پیدا کردن brightest pixel
    glGenTextures(1, &m_lightSourceTex);
    glBindTexture(GL_TEXTURE_2D, m_lightSourceTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 1, 1, 0, GL_RED, GL_FLOAT, &m_settings.lightPosition);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Moon::Rendering::GodRayRenderPass::OcclusionPass(GLuint depthTexture) {
    m_occlusionFBO->Bind();
    m_occlusionFBO->DrawBuffer("Occlusion");
    glClear(GL_COLOR_BUFFER_BIT);

    m_occlusionShader->Bind();
    m_occlusionShader->SetVec3("u_lightPos", m_settings.lightPosition);
    m_occlusionShader->SetInt("u_depthTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    DrawQuad();
    m_occlusionShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_occlusionTex = m_occlusionFBO->GetColorAttachmentHandleByName("Occlusion");
}

void Moon::Rendering::GodRayRenderPass::RadialBlurPass(GLuint occlusionTexture) {
    m_radialBlurShader->Bind();
    GLuint sourceTex = occlusionTexture;

    // Downsample through mips
    for (size_t i = 0; i < m_blurMips.size(); ++i) {
        auto& mip = m_blurMips[i];
        mip.framebuffer->Bind();
        mip.framebuffer->DrawBuffer("Color");
        glViewport(0, 0, mip.width, mip.height);
        glClear(GL_COLOR_BUFFER_BIT);

        m_radialBlurShader->SetVec3("u_lightPos", m_settings.lightPosition);
        m_radialBlurShader->SetFloat("u_density", m_settings.density);
        m_radialBlurShader->SetInt("u_sourceTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTex);

        DrawQuad();

        sourceTex = mip.framebuffer->GetColorAttachmentHandleByName("Color");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_radialBlurShader->Unbind();
    glViewport(0, 0, m_currentWidth, m_currentHeight);
}

void Moon::Rendering::GodRayRenderPass::TemporalAccumulation(GLuint velocityTexture) {
    m_accumulationFBO->Bind();
    m_accumulationFBO->DrawBuffer("Accum");
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_CONSTANT_ALPHA, GL_ONE);
    glBlendColor(0.9f, 0.9f, 0.9f, 1.0f); // 90% history

    m_temporalShader->Bind();
    m_temporalShader->SetInt("u_current", 0);
    m_temporalShader->SetInt("u_history", 1);
    m_temporalShader->SetInt("u_velocity", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_blurMips[0].framebuffer->GetColorAttachmentHandleByName("Color"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_historyTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, velocityTexture);

    DrawQuad();

    m_temporalShader->Unbind();
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Copy to history
    glBindFramebuffer(GL_COPY_READ_BUFFER, m_accumulationFBO->GetHandle());
    glBindFramebuffer(GL_COPY_WRITE_BUFFER, 0);
    glBlitFramebuffer(0, 0, m_currentWidth / 4, m_currentHeight / 4, 0, 0, m_currentWidth / 4, m_currentHeight / 4,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    m_godRayResult = m_accumulationFBO->GetColorAttachmentHandleByName("Accum");
}

void Moon::Rendering::GodRayRenderPass::CompositePass(GLuint targetFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // Additive

    m_compositeShader->Bind();
    m_compositeShader->SetFloat("u_exposure", m_settings.exposure);
    m_compositeShader->SetInt("u_godRayTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_godRayResult);

    DrawQuad();

    m_compositeShader->Unbind();
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::GodRayRenderPass::DrawQuad() {
    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}