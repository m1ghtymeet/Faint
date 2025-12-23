#pragma once

#include <Renderer/Data/FrameDescriptor.h>

namespace Moon::Rendering {

	class CompositeRenderer;

	class ARenderPass {
	public:
		ARenderPass(CompositeRenderer& p_renderer);

		virtual ~ARenderPass() = default;

	protected:
		virtual void OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor);

		virtual void OnEndFrame();

		virtual void Draw() = 0;

	protected:
		CompositeRenderer& m_renderer;
		bool m_enabled = true;

		friend class CompositeRenderer;
	};
}