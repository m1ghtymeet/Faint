#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>
#include <Types/Renderer/TextInstance.h>
#include <UI/Font.h>
#include <memory>

namespace Moon::Rendering {

	class UIRenderPass : public ARenderPass {
	public:
		UIRenderPass(CompositeRenderer& p_renderer);
		virtual void Draw() override;
		unsigned int GetWidgetTextureID();
		unsigned int GetText2DTextureID();

	private:
		glm::mat4 m_projection;
		FrameBuffer* m_framebuffer = nullptr;
	};
}