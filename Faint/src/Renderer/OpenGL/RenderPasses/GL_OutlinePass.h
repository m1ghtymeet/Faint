#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Scene/Entity.h>

namespace Moon::Rendering {

	class OutlineRenderPass : public ARenderPass {
	public:
		OutlineRenderPass(CompositeRenderer& p_renderer);

		void SetHoveredEntity(Moon::Entity* p_entity);

	private:
		virtual void Draw() override;
		Moon::Entity* m_hoveredEntity = nullptr;
	};
}