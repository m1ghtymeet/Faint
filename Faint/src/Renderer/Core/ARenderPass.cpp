#include "ARenderPass.h"

Moon::Rendering::ARenderPass::ARenderPass(CompositeRenderer& p_renderer) :
	m_renderer(p_renderer)
{
}

void Moon::Rendering::ARenderPass::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
}

void Moon::Rendering::ARenderPass::OnEndFrame()
{
}
