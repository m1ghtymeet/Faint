#include "GL_UIPass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/Shader.h>

#include <UI/UIBackEnd.h>

Moon::Rendering::UIRenderPass::UIRenderPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer) {

	m_framebuffer = new FrameBuffer("UI", 1600, 900);
	m_framebuffer->CreateAttachment("Color", GL_RGBA16F);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::UIRenderPass::Draw() {
	float width = m_renderer.GetFrameDescriptor().renderWidth;
	float height = m_renderer.GetFrameDescriptor().renderHeight;

	// Setup projection
	m_projection = glm::ortho(
		0.0f,
		width,
		height,
		0.0f,
		-1000.0f, 1000.0f);

	m_framebuffer->Resize(width, height);

	m_framebuffer->Bind();
	m_framebuffer->DrawBuffer("Color");
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	//Shader* uiShader = m_renderer.GetShader("ui").get();
	//uiShader->Bind();
	//uiShader->SetMat4("u_projection", m_projection);
	//uiShader->Unbind();

	for (auto& text : m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene->GetFastAccessComponents().texts) {
		if (!text->owner.IsActive()) continue;
		glm::vec3 pos = text->owner.transform->GetGlobalPosition();
		glm::vec3 color = text->GetColor();
		UIBackEnd::BlitText(text->GetText(), text->GetFont(), text->GetFontSize(), pos, color);
	}
	auto& uiMesh = UIBackEnd::GetUIMesh();
	auto& renderItems = UIBackEnd::GetRenderItems();
	if (renderItems.empty()) return;

	auto shader = m_renderer.GetShader("text2d");

	//glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader->Bind();
	shader->SetFloat("u_pxRange", 6.0f);
	shader->SetMat4("u_projection", m_projection); // Ortho projection
	glBindVertexArray(uiMesh.GetVAO());
	for (const auto& item : renderItems) {
		// Bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, item.textureIndex);
		shader->SetInt("u_atlasTexture", 0);
		// Set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, item.filter == 0 ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, item.filter == 0 ? GL_LINEAR : GL_NEAREST);
		// Handle clipping if set (scissor test)
		if (item.clipMinX != -1 && item.clipMinY != -1 && item.clipMaxX != -1 && item.clipMaxY != -1) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(item.clipMinX, item.clipMinY, item.clipMaxX - item.clipMinX, item.clipMaxY - item.clipMinY);
		}
		// Draw
		glDrawElementsBaseVertex(GL_TRIANGLES, item.indexCount, GL_UNSIGNED_INT, (void*)(item.baseIndex * sizeof(uint32_t)), item.baseVertex);
		if (item.clipMinX != -1) glDisable(GL_SCISSOR_TEST);
	}
	glBindVertexArray(0);
	shader->Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

unsigned int Moon::Rendering::UIRenderPass::GetWidgetTextureID() {
	return m_framebuffer->GetColorAttachmentHandleByName("Color");
}

unsigned int Moon::Rendering::UIRenderPass::GetText2DTextureID() {
	return m_framebuffer->GetColorAttachmentHandleByName("Color");
}
