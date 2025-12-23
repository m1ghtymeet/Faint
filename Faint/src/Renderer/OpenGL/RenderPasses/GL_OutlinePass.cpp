#include "GL_OutlinePass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/Shader.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <AssetManagment/MeshManager.h>

#include <Input/Input.h>
#include <Input/KeyCodes.h>

namespace {
	Moon::Shader* shader = nullptr;
	Moon::Mesh* mesh = nullptr;
}

Moon::Rendering::OutlineRenderPass::OutlineRenderPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer)
{
	shader = new Shader({ "data/shaders/outline.vert", "data/shaders/outline.frag" });

	mesh = AssetManagment::MeshManager::GetQuad();
}

void Moon::Rendering::OutlineRenderPass::SetHoveredEntity(Moon::Entity* p_entity) {
	m_hoveredEntity = p_entity;
}

void Moon::Rendering::OutlineRenderPass::Draw() {
	
	return;
	if (m_hoveredEntity == nullptr) return;

	auto framebuffer = m_renderer.GetFrameBuffer("GBuffer");
	framebuffer->Bind();
	framebuffer->DrawBuffer("FinalLighting");

	if (Input::KeyPressed(Key::T))
		shader->Hotload();

	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	shader->Bind();
	shader->SetVec3("color", glm::vec3(1, 0, 0));
	shader->SetMat4("u_modelMatrix", m_hoveredEntity->transform->GetGlobalMatrix());
	shader->SetVec2("u_screenSize", { m_renderer.GetFrameDescriptor().renderWidth, m_renderer.GetFrameDescriptor().renderHeight });

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer->GetDepthAttachmentHandle());
	//m_renderer.DrawEntity(m_hoveredEntity);
	//mesh->Bind();
	//glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
	//mesh->Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
