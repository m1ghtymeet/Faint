#include "GL_SkyboxPass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <AssetManagment/Loader/ModelLoader.h>
#include <AssetManagment/MeshManager.h>
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Scene/Components/CDirectionalLight.h>

Moon::Rendering::SkyboxRenderPass::SkyboxRenderPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer) {

}

Moon::Rendering::SkyboxRenderPass::~SkyboxRenderPass() {

}

void Moon::Rendering::SkyboxRenderPass::Draw() {
	RenderSkybox();
}

void Moon::Rendering::SkyboxRenderPass::SetTimeOfDay(float hours) {
	m_settings.timeOfDay = fmod(hours, 24.0f);
}

void Moon::Rendering::SkyboxRenderPass::UpdateSunDirection(float azimuth, float elevation) {
	float az = glm::radians(azimuth);
	float el = glm::radians(elevation);

	m_settings.sunDirection = glm::vec3(
		cos(el) * sin(az),
		sin(el),
		cos(el) * cos(az)
	);
}

void Moon::Rendering::SkyboxRenderPass::RenderSkybox() {
    Shader* shader = m_renderer.GetShader("procedularSky").get();
    FrameBuffer* framebuffer = m_renderer.GetFrameBuffer("GBuffer").get();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    framebuffer->Bind();
    framebuffer->DrawBuffer("FinalLighting");
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    shader->Bind();
    
    const auto& scene = m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene;
    const auto& fastAccess = scene->GetFastAccessComponents();
    Light lightData;
    glm::quat dir = glm::quat();
    for (auto light : fastAccess.lights) {
        if (light->owner.IsActive()) {
            if (dynamic_cast<DirectionalLightComp*>(light)) {
                lightData = light->GetData();
                dir = light->owner.transform->GetGlobalRotation();
            }
        }
    }

    shader->SetVec3("u_sunDirection", lightData.GetDirectionFromQuat(dir));
    shader->SetVec3("u_sunColor", lightData.color);
    
    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());

    auto mesh = AssetManagment::MeshManager::GetMeshByName("Cube");
    glDrawElementsBaseVertex(GL_TRIANGLES,
        mesh->indexCount,
        GL_UNSIGNED_INT,
        (void*)(uintptr_t)(mesh->firstIndex * sizeof(uint32_t)),
        mesh->baseVertex);
    glBindVertexArray(0);
    
    shader->Unbind();
    
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
