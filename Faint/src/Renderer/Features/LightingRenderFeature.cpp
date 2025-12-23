#include "LightingRenderFeature.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/GL_SSBO.hpp>

#include <Scene/Components/CDirectionalLight.h>

namespace {
	OpenGLSSBO* lightSSBO = nullptr;
}

Moon::Rendering::LightingRenderFeature::LightingRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy)
	: ARenderFeature(p_renderer, p_executionPolicy) {
	// Each Lightdata in 96 bytes, max 64 lights for now
	lightSSBO = new OpenGLSSBO(sizeof(LightData) * 64, GL_DYNAMIC_STORAGE_BIT);
}

int Moon::Rendering::LightingRenderFeature::GetLightCount() const {
	return m_lightCount;
}

void Moon::Rendering::LightingRenderFeature::OnBeginFrame(const Data::FrameDescriptor& frameDescriptor) {
	std::vector<Rendering::LightData> lights;
	lights.reserve(64);

	for (auto lightComp : m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene->GetFastAccessComponents().lights) {
		TransformComponent* transform = lightComp->owner.transform;
		LightType componentType = LightType::Point;
		if (dynamic_cast<DirectionalLightComp*>(lightComp)) {
			componentType = LightType::Directional;
		}
		const Light& lightData = lightComp->GetData();
		LightData gpuLight = lightData.PackLightData(
			componentType,
			transform->GetGlobalPosition(),
			transform->GetGlobalRotation()
		);
		gpuLight.type = componentType;
		lights.push_back(gpuLight);
	}
	if (!lights.empty()) {
		m_lightCount = static_cast<int>(lights.size());
		lightSSBO->Update(lights.size() * sizeof(Rendering::LightData), (void*)lights.data());
		lightSSBO->Bind(1);
	}
	else {
		m_lightCount = 0;
	}
}
