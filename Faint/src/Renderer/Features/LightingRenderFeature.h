#pragma once

#include <Renderer/Features/ARenderFeature.h>

namespace Moon::Rendering {
	
	class LightingRenderFeature : public ARenderFeature {
	public:
		LightingRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy = FeatureExecutionPolicy::ALWAYS);
		
		int GetLightCount() const;

	private:
		virtual void OnBeginFrame(const Data::FrameDescriptor& frameDescriptor) override;
		int m_lightCount = 0;
	};
}