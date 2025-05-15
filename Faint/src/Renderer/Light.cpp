#include "Light.h"

namespace Faint {

	DirectionalLight::DirectionalLight()
	{
		for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
		{

		}
	}

	void DirectionalLight::CSMViewCalculation(const Matrix4& camView, const Matrix4& camProjection)
	{
		Matrix4 viewProjection = camProjection * camView;
		Matrix4 inverseViewProjection = glm::inverse(viewProjection);

		// Calculate the optimal cascade distances
		const float range = CSM_FAR_CLIP - CSM_NEAR_CLIP;
		const float ratio = CSM_FAR_CLIP / CSM_NEAR_CLIP;
		for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
		{
			const float p = (i + 1) / static_cast<float>(4);
			const float log = CSM_NEAR_CLIP * glm::pow(ratio, p);
			const float uniform = CSM_NEAR_CLIP + range * p;
			const float d = 0.91f * (log - uniform) + uniform;
			m_CascadeSplits[i] = (d - CSM_NEAR_CLIP) / CSM_CLIP_RANGE;
		}

		float lastSplitDist = 0.0f;
	}
}
