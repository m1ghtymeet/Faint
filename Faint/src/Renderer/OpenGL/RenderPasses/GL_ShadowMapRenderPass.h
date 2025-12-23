#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Scene/Entity.h>

namespace Moon::Rendering {
	class ShadowMapRenderPass : public ARenderPass {
	public:
		ShadowMapRenderPass(CompositeRenderer& p_renderer);

		unsigned int GetID() const;

	private:
		virtual void Draw() override;
	};
}