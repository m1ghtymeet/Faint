#include "GL_LightingPass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/Shader.h>

#include <Renderer/Features/DebugLineRenderFeature.h>
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/RenderPasses/GL_ShadowMapRenderPass.h>
#include <AssetManagment/MeshManager.h>

#include <Scene/Components/CDirectionalLight.h>

//namespace {
//	Moon::Mesh* mesh = nullptr;
//}

Moon::Rendering::LightingRenderPass::LightingRenderPass(CompositeRenderer& p_renderer)
	: ARenderPass(p_renderer)
{
	//mesh = AssetManagment::MeshManager::GetQuad();
}

void Moon::Rendering::LightingRenderPass::Draw() {
	FrameBuffer* fb = m_renderer.GetFrameBuffer("GBuffer").get();
	Shader* shader = m_renderer.GetShader("lighting").get();

	fb->Bind();
	fb->DrawBuffer("FinalLighting");

	shader->Bind();
	shader->SetMat4("u_invView", m_renderer.GetFrameDescriptor().camera->GetInverseViewMatrix());
	shader->SetVec3("u_viewPos", m_renderer.GetFrameDescriptor().camera->GetInverseViewMatrix()[3]);

	//if (m_renderer.HasDescriptor<SceneRenderer::RendererDescriptor>())
	//	shader->SetInt("u_rendererOrder", m_renderer.GetDescriptor<SceneRenderer::RendererDescriptor>().order);
	
	glBindTextureUnit(0, fb->GetColorAttachmentHandleByName("Albedo"));
	glBindTextureUnit(1, fb->GetColorAttachmentHandleByName("Normal"));
	glBindTextureUnit(2, fb->GetColorAttachmentHandleByName("RMA"));
	glBindTextureUnit(3, fb->GetColorAttachmentHandleByName("WorldSpacePosition"));

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderer.GetPass<ShadowMapRenderPass>("ShadowMap").GetID());
	
	//glBindImageTexture(
	//	0,
	//	fb->GetColorAttachmentHandleByName("FinalLighting"),
	//	0,
	//	GL_FALSE,
	//	0,
	//	GL_READ_WRITE,
	//	GL_RGB16F
	//);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t Moon::Rendering::LightingRenderPass::GetID() {
	return m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("FinalLighting");
}
