#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>

namespace Moon::Rendering {

	class LightingRenderPass : public ARenderPass {
	public:
		LightingRenderPass(CompositeRenderer& p_renderer);

		uint32_t GetID();

	private:
		virtual void Draw() override;
	};
}