#include "GL_ShadowMapRenderPass.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/Shader.h>

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024
#define MAX_SHADOW_CASTING_LIGHTS 12

namespace Moon::Rendering {
	Shader* shadowShader = nullptr;
	std::vector<glm::mat4> shadowTransforms;

	unsigned int depthMapFBO;
	unsigned int depthMap;

	struct ShadowCache {
		glm::vec3 lastLightPos;
		bool needsUpdate;
		int framesSinceUpdate;
	};
	std::unordered_map<uint32_t, ShadowCache> shadowCaches;
}

Moon::Rendering::ShadowMapRenderPass::ShadowMapRenderPass(CompositeRenderer& p_renderer)
	: ARenderPass(p_renderer) {
	shadowShader = m_renderer.GetShader("shadowCubeMap").get();

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
	for (unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_DEPTH_COMPONENT16,
			SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowTransforms.reserve(6);
}

unsigned int Moon::Rendering::ShadowMapRenderPass::GetID() const {
	return depthMap;
}

void Moon::Rendering::ShadowMapRenderPass::Draw() {
	//const auto& scene = m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene;
	//int shadowCastingLights = 0;
	//
	//for (auto lights : scene->GetFastAccessComponents().lights) {
	//	if (shadowCastingLights >= MAX_SHADOW_CASTING_LIGHTS) break;
	//
	//	auto& transform = lights->owner.transform;
	//	Light& light = lights->GetData();
	//
	//	if (light.castShadows) {
	//		if (light.type == LightType::Point) {
	//			uint32_t lightID = lights->owner.GetID();
	//			glm::vec3 lightPos = transform->GetGlobalPosition();
	//
	//			auto& cache = shadowCaches[lightID];
	//			cache.framesSinceUpdate++;
	//			
	//			float moveDist = glm::distance(cache.lastLightPos, lightPos);
	//			if (moveDist < 0.1f && cache.framesSinceUpdate < 3) {
	//				continue;
	//			}
	//			
	//			cache.lastLightPos = lightPos;
	//			cache.framesSinceUpdate = 0;
	//
	//			glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	//			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//			glEnable(GL_DEPTH_TEST);
	//
	//			glDrawBuffer(GL_NONE);
	//			glReadBuffer(GL_NONE);
	//
	//			float near_plane = 0.1f;
	//			float far_plane = light.radius;
	//			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
	//
	//			shadowTransforms.clear();
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)));
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)));
	//			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));
	//
	//			auto& geometry = m_renderer.GetPass<GeometryRenderPass>("Geometry");
	//			shadowShader->Bind();
	//			for (unsigned int face = 0; face < 6; ++face) {
	//				shadowShader->SetMat4("shadowMatrices[" + std::to_string(face) + "]", shadowTransforms[face]);
	//			}
	//			shadowShader->SetVec3("lightPos", lightPos);
	//			shadowShader->SetFloat("far_plane", far_plane);
	//
	//			for (unsigned int face = 0; face < 6; ++face) {
	//				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthMap, 0);
	//				glClear(GL_DEPTH_BUFFER_BIT);
	//				shadowShader->SetInt("faceIndex", face);
	//				//for (RenderItem& renderItem : geometry.m_renderQueue.opaque) {
	//				//	renderItem.mesh->Bind();
	//				//
	//				//	glm::vec3 objPos = renderItem.transform.GetWorldPosition();
	//				//	float dist = glm::distance(objPos, lightPos);
	//				//	if (dist > far_plane + 5.0f) continue;
	//				//
	//				//	shadowShader->SetMat4("modelMatrix", renderItem.transform.GetWorldMatrix());
	//				//	glDrawElements(GL_TRIANGLES, renderItem.mesh., GL_UNSIGNED_INT, 0);
	//				//	renderItem.mesh->Unbind();
	//				//}
	//			}
	//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//			shadowCastingLights++;
	//		}
	//		else if (light.type == LightType::Directional) {
	//			//glViewport(0, 0, 1024, 1024);
	//			//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//			//glClear(GL_DEPTH_BUFFER_BIT);
	//			//float near_plane = 1.0f, far_plane = 7.5f;
	//			//glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	//			//glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
	//			//	glm::vec3(0.0f, 0.0f, 0.0f),
	//			//	glm::vec3(0.0f, 1.0f, 0.0f));
	//			//glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	//			//shadowShader->Bind();
	//			//shadowShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
	//			//
	//			//auto& geometry = m_renderer.GetPass<GeometryRenderPass>("Geometry");
	//			////glDisable(GL_BLEND);
	//			////glDisable(GL_CULL_FACE);
	//			////glEnable(GL_DEPTH_TEST);
	//			////glDepthMask(GL_TRUE);
	//			//for (RenderItem& renderItem : geometry.m_queues.opaque) {
	//			//	renderItem.mesh->Bind();
	//			//	shadowShader->SetMat4("modelMatrix", renderItem.transform->GetWorldMatrix());
	//			//	glDrawElements(GL_TRIANGLES, renderItem.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
	//			//	renderItem.mesh->Unbind();
	//			//}
	//			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//		}
	//	}
	//}
	//
	//glViewport(0, 0, m_renderer.GetFrameDescriptor().renderWidth, m_renderer.GetFrameDescriptor().renderHeight);
}
