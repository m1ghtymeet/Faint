#include "SceneRenderer.h"

#include <AssetManagment/MeshManager.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <Renderer/Features/EntityRenderFeature.h>
#include <Renderer/Features/LightingRenderFeature.h>
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/GL_SSBO.hpp>
#include <iostream>

namespace Moon::Rendering {
	class SceneRenderPass : public ARenderPass {
	public:
		SceneRenderPass(CompositeRenderer& p_renderer) :
			ARenderPass(p_renderer) {
		}
	};

	class OpaqueRenderPass : public ARenderPass {
	public:
		OpaqueRenderPass(CompositeRenderer& p_renderer) :
			ARenderPass(p_renderer)
		{
		}
	protected:
		virtual void Draw() override {
			// ZoneScoped
			FrameBuffer* framebuffer = m_renderer.GetFrameBuffer("GBuffer").get();
			framebuffer->Bind();
			framebuffer->DrawBuffers(
				{ "Albedo", "Normal", "RMA", "WorldSpacePosition", "ScreenSpacePosition", "MousePick" }
			);

			const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredRenderItemsDescriptor>();
			glBindVertexArray(Rendering::OpenGLBackEnd::GetVertexDataVAO());
			for (const auto& item : drawables.opaques | std::views::values) {
				m_renderer.DrawEntity(PipelineState{}, item);
			}
			glBindVertexArray(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};

	class TransparentRenderPass : public ARenderPass {
	};
}

using namespace Moon::Rendering;

Moon::Rendering::SceneRenderer::SceneRenderer() {

	auto gBuffer = CreateFrameBuffer("GBuffer", 1600, 900);
	gBuffer->CreateAttachment("Albedo", GL_RGBA16F);
	gBuffer->CreateAttachment("Normal", GL_RGBA16F);
	gBuffer->CreateAttachment("RMA", GL_RGBA16F);
	gBuffer->CreateAttachment("FinalLighting", GL_RGBA16F, GL_LINEAR, GL_LINEAR);
	gBuffer->CreateAttachment("WorldSpacePosition", GL_RGBA16F);
	gBuffer->CreateAttachment("ScreenSpacePosition", GL_RGBA16F);
	gBuffer->CreateAttachment("MousePick", GL_R32I);
	gBuffer->CreateDepthAttachment(GL_DEPTH32F_STENCIL8);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CreateShader("default",		       { "data/shaders/default.ftshader" });
	CreateShader("gBuffer",		       { "data/shaders/gBuffer.ftshader" });
	CreateShader("lighting",	       { "data/shaders/lighting.mnshader" });
	CreateShader("shadowCubeMap",      { "data/shaders/shadow_map.ftshader" });
	CreateShader("ui",				   { "data/shaders/ui.mnshader" });
	CreateShader("text2d",			   { "data/shaders/text2d.ftshader" });
	CreateShader("postProcessing",     { "data/shaders/postProcessing.ftshader" });
	CreateShader("ssao",		       { "data/shaders/ssao.ftshader" });
	CreateShader("ssao_blur",	       { "data/shaders/ssao_blur.ftshader" });
	CreateShader("procedularSky",      { "data/shaders/procedular_sky.ftshader" });
	CreateShader("bloom_prefilter",    { "data/shaders/fx/bloom_prefilter.mnshader" });
	CreateShader("bloom_downsample",   { "data/shaders/fx/bloom_downsample.mnshader" });
	CreateShader("bloom_upsample",     { "data/shaders/fx/bloom_upsample.mnshader" });
	CreateShader("bloom_composite",    { "data/shaders/fx/bloom_composite.mnshader" });
	CreateShader("godray_detect",	   { "data/shaders/fx/godray_detect.mnshader" });
	CreateShader("godray_occlusion",   { "data/shaders/fx/godray_occlusion.mnshader" });
	CreateShader("godray_radial_blur", { "data/shaders/fx/godray_radial_blur.mnshader" });
	CreateShader("godray_temporal",	   { "data/shaders/fx/godray_temporal.mnshader" });
	CreateShader("godray_composite",   { "data/shaders/fx/godray_composite.mnshader" });

	AddFeature<EntityRenderFeature>();
	AddFeature<LightingRenderFeature>();

	AddPass<OpaqueRenderPass>("Opaques", /*20000U*/1);
	//AddPass<TransparentRenderPass>("Transparents", 30000U);
}

void Moon::Rendering::SceneRenderer::BeginFrame(const Data::FrameDescriptor& p_frameData) {
	FT_CORE_ASSERT(HasDescriptor<SceneDescriptor>(), "Cannot find SceneDescriptor attached to this renderer");

	GetFrameBuffer("GBuffer")->Bind();
	GetFrameBuffer("GBuffer")->ClearAttachment("Albedo", 0, 0, 0, 0);
	GetFrameBuffer("GBuffer")->ClearAttachment("Normal", 0, 0, 0, 0);
	GetFrameBuffer("GBuffer")->ClearAttachment("RMA", 0, 0, 0, 0);
	GetFrameBuffer("GBuffer")->ClearAttachment("WorldSpacePosition", 0, 0, 0, 0);
	GetFrameBuffer("GBuffer")->ClearAttachment("ScreenSpacePosition", 0, 0, 0, 0);
	GetFrameBuffer("GBuffer")->ClearAttachmentI("MousePick", 0);

	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	AddDescriptor<SceneRenderItemsDescriptor>(ParseScene(*GetDescriptor<SceneDescriptor>().scene));
	AddDescriptor<SceneFilteredRenderItemsDescriptor>(FilterRenderItems(GetDescriptor<SceneRenderItemsDescriptor>()));

	CompositeRenderer::BeginFrame(p_frameData);
}

Moon::Rendering::SceneRenderer::SceneRenderItemsDescriptor SceneRenderer::ParseScene(const Scene& scene) {
	SceneRenderItemsDescriptor result;

	for (const auto meshRenderer : scene.GetFastAccessComponents().modelRenderers) {
		auto& owner = meshRenderer->owner;
		if (!owner.IsActive()) continue;
		const auto model = meshRenderer->GetModel();
		if (!model) continue;
		const auto materialRenderer = owner.GetComponent<MaterialRendererComp>();
		if (!materialRenderer) continue;

		const auto& materials = materialRenderer->GetMaterials();
		const auto& transform = meshRenderer->owner.transform->GetTransform();

		for (auto& mesh : model->GetMeshIndices()) {
			uint32_t materialIndex = Moon::AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex;
			if (materialIndex >= materials.size() || !materials[materialIndex])
				continue;

			auto material = materials[materialIndex];

			RenderItem renderItem{
				.meshIndex = mesh,
				.material = material,
				.transform = transform,
				.id = (uint32_t)owner.GetID(),
				.distanceToCamera = 1
			};
			result.renderItems.push_back(renderItem);
		}
	}

	return result;
}

Moon::Rendering::SceneRenderer::SceneFilteredRenderItemsDescriptor SceneRenderer::FilterRenderItems(const Moon::Rendering::SceneRenderer::SceneRenderItemsDescriptor& renderItems) {
	SceneFilteredRenderItemsDescriptor output;
	for (const auto& renderItem : renderItems.renderItems) {
		// Calculate distance for transparent sorting
		float distance = 0.0f;
		if (renderItem.material->IsBlendable()) {
			glm::vec3 worldPos = glm::vec3(renderItem.transform.GetWorldMatrix()[3]);
			distance = glm::distance2(GetFrameDescriptor().camera->GetPosition(), worldPos);
		}

		auto renderItemCopy = renderItem;

		if (renderItemCopy.material->IsBlendable()) {
			
		}
		else {
			output.opaques.emplace(decltype(decltype(output.opaques)::value_type::first){
				.order = 1,
					.distance = distance
			}, renderItemCopy);
		}
	}
	// Sort opaque by material/shader to minimize state changes
	//std::sort(output.opaques.begin(), output.opaques.end(),
	//	[](const RenderItem& a, const RenderItem& b) {
	//		if (a.material->GetShader() != b.material->GetShader())
	//			return a.material->GetShader() < b.material->GetShader();
	//		if (a.material != b.material)
	//			return a.material < b.material;
	//		return a.meshIndex < b.meshIndex;
	//	});

	return output;
}