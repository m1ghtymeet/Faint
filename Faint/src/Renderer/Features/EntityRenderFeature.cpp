#include "EntityRenderFeature.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/GL_SSBO.hpp>

namespace Moon::Rendering {
	OpenGLSSBO* cameraSSBO = nullptr;

	OpenGLSSBO* globalSSBO = nullptr;

	struct GlobalDataInfo {
		int lightCount;
	};

	std::vector<Moon::Rendering::Light> FindActiveLights(const Scene& p_scene) {
		std::vector<Moon::Rendering::Light> lights;
		const auto& fastAccess = p_scene.GetFastAccessComponents();
		for (auto light : fastAccess.lights) {
			if (light->owner.IsActive()) {
				lights.push_back(light->GetData());
			}
		}
		return lights;
	}
}

Moon::Rendering::EntityRenderFeature::EntityRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy) :
	ARenderFeature(p_renderer, p_executionPolicy)
{
	cameraSSBO = new OpenGLSSBO(sizeof(CameraDescriptor), GL_DYNAMIC_STORAGE_BIT);
	globalSSBO = new OpenGLSSBO(sizeof(GlobalDataInfo), GL_DYNAMIC_STORAGE_BIT);
}

void Moon::Rendering::EntityRenderFeature::SetCamera(const ICamera& p_camera) {

	CameraDescriptor cameraDesc = {
		.projection = p_camera.GetProjectionMatrix(),
		.view = p_camera.GetViewMatrix()
	};
	if (cameraSSBO) {
		cameraSSBO->Update(sizeof(CameraDescriptor), &cameraDesc);
		cameraSSBO->Bind(0);
	}
}

void Moon::Rendering::EntityRenderFeature::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor) {
	
	SetCamera(*p_frameDescriptor.camera);
	
	m_lightCount = static_cast<int>(FindActiveLights(*m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene).size());
	GlobalDataInfo globalInfo = { .lightCount = m_lightCount };

	if (globalSSBO) {
		globalSSBO->Update(sizeof(GlobalDataInfo), &globalInfo);
		globalSSBO->Bind(2);
	}
}
