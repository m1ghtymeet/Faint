#include "ARenderFeature.h"

bool Moon::Rendering::ARenderFeature::IsEnabled() const {
    return true;
}

//Moon::Rendering::ARenderFeature& Moon::Rendering::ARenderFeature::Include(std::type_index p_type) {
//    return ARenderFeature();
//}

Moon::Rendering::ARenderFeature::ARenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy) :
    m_renderer(p_renderer)
{
}

void Moon::Rendering::ARenderFeature::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
}

void Moon::Rendering::ARenderFeature::OnEndFrame()
{
}

void Moon::Rendering::ARenderFeature::OnBeforeDraw()
{
}
