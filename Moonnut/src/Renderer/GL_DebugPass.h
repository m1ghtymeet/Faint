#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Scene/Components/CBoxCollider.h>

namespace Moon {
	class Entity;
	class Camera;
}

namespace Moon::Rendering {

	class DebugRenderPass : public ARenderPass {
	public:
		DebugRenderPass(CompositeRenderer& p_renderer);

		Entity* selected = nullptr;

	private:
		virtual void Draw() override;
		void DrawGrid();
		void DrawEntityDebugElements(Entity& entity);
	};
}