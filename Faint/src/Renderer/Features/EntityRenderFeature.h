#pragma once

#include <Renderer/Features/ARenderFeature.h>

namespace Moon::Rendering {
	
	struct CameraDescriptor {
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct EntityDescriptor {
		glm::mat4 modelMatrix;
		int id;
	};

	class EntityRenderFeature : public ARenderFeature {
	public:
		EntityRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy = FeatureExecutionPolicy::WHITELIST_ONLY);

		void SetCamera(const ICamera& p_camera);

	protected:
		virtual void OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor) override;
		
	private:
		int m_lightCount = 0;
	};
}