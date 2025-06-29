#include "OpenGLRenderer.h"
#include "Renderer/Renderer.h"
#include "Renderer/OpenGL/Types/ShadowMap.h"
#include "Renderer/OpenGL/Types/GL_debug_mesh.hpp"
#include "Renderer/OpenGL/Types/CubemapTexture.h"
#include "Renderer/OpenGL/Types/CubemapView.h"

#include "Engine.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "AssetManagment/Config.h"
#include "Util/Util.h"

#define SHADOW_MAP_SIZE 2048

namespace Faint::OpenGLRenderer {
	std::unordered_map<std::string, Shader> g_shaders;
	std::unordered_map<std::string, FrameBuffer> g_frameBuffers;
	std::unordered_map<std::string, CubemapTexture> g_cubemapTextures;
	std::unordered_map<std::string, OpenGLCubemapView> g_cubemapViews;
	std::unordered_map<std::string, OpenGLSSBO> g_ssbos;
	int lightCount, materialCount;

	glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
	glm::mat4 lightView = glm::lookAt(20.0f * glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 20.5f);
	std::vector<glm::mat4> shadowTransforms;

	//ShadowMap* m_ShadowMap;
	Matrix4 m_Projection, m_View;
	Vec3 m_CamPos;
	Ref<Mesh> cubeMesh;

	Vec2 lastResolution = Vec2();

	unsigned int depthMapFBO;
	unsigned int depthMap;
	unsigned int depthCubeMapFBO;
	unsigned int depthCubeMap;

	void ClearRenderTargets();

	void Init() {
		g_frameBuffers["GBuffer"] = FrameBuffer("GBuffer", 1280, 720);
		g_frameBuffers["GBuffer"].CreateAttachment("BaseColor", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Normal", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("RMA", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Lighting", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("WorldSpacePosition", GL_RGBA32F);
		g_frameBuffers["GBuffer"].CreateAttachment("MousePick", GL_R32I);
		g_frameBuffers["GBuffer"].CreateDepthAttachment(GL_DEPTH_COMPONENT32F);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		g_frameBuffers["FinalImage"] = FrameBuffer("FinalImage", Config::finalImageResolution);
		g_frameBuffers["FinalImage"].CreateAttachment("Color", GL_RGB16F);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		g_frameBuffers["UI"] = FrameBuffer("UI", 1920, 1080);
		g_frameBuffers["UI"].CreateAttachment("Color", GL_RGBA8, GL_NEAREST, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));

		glGenFramebuffers(1, &depthMapFBO);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glGenFramebuffers(1, &depthCubeMapFBO);
		//glGenTextures(1, &depthCubeMap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
		//for (unsigned int i = 0; i < 6; ++i) {
		//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
		//		2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		//}
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
		//glReadBuffer(GL_NONE);
		//glDrawBuffer(GL_NONE);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate debug meshes
		std::vector<Vertex> lineVertices{
			{ Vec3(0, 0, 0), Vec3(0, 0, 0), Vec2(0, 0) },
			{ Vec3(1, 1, 1), Vec3(0, 0, 0), Vec2(0, 0) }
		};
		std::vector<uint32_t> lineIndices{ 0, 1 };

		g_ssbos["Lights"] = OpenGLSSBO(sizeof(Light) * MAX_LIGHTS, GL_DYNAMIC_STORAGE_BIT);
		g_ssbos["Materials"] = OpenGLSSBO(sizeof(MaterialData) * 120, GL_DYNAMIC_STORAGE_BIT);

		//g_cubemapTextures["NightSky"] = CubemapTexture();
		//g_cubemapTextures["NightSky"].Load("data/textures/NightSky");

		LoadShaders();
	}

	void LoadShaders() {
		g_shaders["gBuffer"] = Shader({ "data/shaders/gBuffer.vert", "data/shaders/gBuffer.frag" });
		g_shaders["deferred"] = Shader({ "data/shaders/deferred.vert", "data/shaders/deferred.frag" });
		g_shaders["ShadowMap"] = Shader({ "data/shaders/direct_shadow.vert", "data/shaders/direct_shadow.frag" });
		g_shaders["PointShadowMap"] = Shader({ "data/shaders/shadowMap.vert", "data/shaders/shadowMap.frag", "data/shaders/shadowMap.geom" });
		g_shaders["skybox"] = Shader({ "data/shaders/skybox.vert", "data/shaders/skybox.frag" });
		g_shaders["DebugVertex"] = Shader({ "data/shaders/debug_vertex.vert", "data/shaders/debug_vertex.frag" });
		g_shaders["Outline"] = Shader({ "data/shaders/outline.vert", "data/shaders/outline.frag" });
	}

	void Cleanup() {
	}

	void Update(const Time time) {
		if (Engine::IsPlayMode()) {

		}
	}

	void UpdateSSBOS() {
		std::vector<Light> lights{};
		auto view = Engine::GetCurrentScene()->Reg().view<TransformComponent, LightComponent>();
		for (auto e : view) {
			auto [transform, lightComponent] = view.get<TransformComponent, LightComponent>(e);
			Light light;
			light.posX = transform.GetGlobalPosition().x; light.posY = transform.GetGlobalPosition().y;
			light.posZ = transform.GetGlobalPosition().z; light.colorR = lightComponent.Color.r;
			light.colorG = lightComponent.Color.g; light.colorB = lightComponent.Color.b;
			light.radius = lightComponent.Radius; light.strength = lightComponent.Strength;
			light.type = (int)lightComponent.Type;
			lights.push_back(light);
		}
		if (!lights.empty()) {
			g_ssbos["Lights"].Update(lights.size() * sizeof(Light), (void*)&lights[0]);
			g_ssbos["Lights"].Bind(1);
			lightCount = (int)lights.size();
		}

		std::vector<MaterialData> materials{};
		auto matView = Engine::GetCurrentScene()->Reg().view<MeshRendererComponent>();
		//for (auto e : matView) {
		//	MeshRendererComponent& meshComponent = matView.get<MeshRendererComponent>(e);
		//	for (Ref<Mesh> mesh : meshComponent._Model->GetMeshes()) {
		//		materials.push_back(mesh->GetMaterial()->data);
		//		mesh->GetMaterial()->id++;
		//	}
		//}
		//if (!materials.empty()) {
		//	std::cout << materials.size() << '\n';
		//	g_ssbos["Materials"].Update(materials.size() * sizeof(MaterialData), (void*)&materials[0]);
		//	g_ssbos["Materials"].Bind(2);
		//}
	}

	void CreateSSBO(const std::string& name, float size, GLbitfield flags) {
		g_ssbos[name] = OpenGLSSBO(size, flags);
	}

	void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos) {
		m_Projection = projection;
		m_View = view;
		m_CamPos = camPos;
	}

	void DebugDrawWireSphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 32, int rings = 8) {
		for (int i = 0; i <= rings; ++i) {
			float phi = glm::pi<float>() * float(i) / float(rings); // ÒÇæ?å È?ä 0 ÊÇ pi
			float y = cos(phi) * radius;
			float ringRadius = sin(phi) * radius;

			for (int j = 0; j < segments; ++j) {
				float theta1 = glm::two_pi<float>() * float(j) / float(segments);
				float theta2 = glm::two_pi<float>() * float(j + 1) / float(segments);

				glm::vec3 p1 = center + glm::vec3(cos(theta1) * ringRadius, y, sin(theta1) * ringRadius);
				glm::vec3 p2 = center + glm::vec3(cos(theta2) * ringRadius, y, sin(theta2) * ringRadius);

				DrawLine(p1, p2, color);
			}
		}

		for (int j = 0; j < segments; ++j) {
			float theta = glm::two_pi<float>() * float(j) / float(segments);
			float nextTheta = glm::two_pi<float>() * float(j + 1) / float(segments);

			for (int i = 0; i < rings; ++i) {
				float phi1 = glm::pi<float>() * float(i) / float(rings);
				float phi2 = glm::pi<float>() * float(i + 1) / float(rings);

				glm::vec3 p1 = center + glm::vec3(
					sin(phi1) * cos(theta) * radius,
					cos(phi1) * radius,
					sin(phi1) * sin(theta) * radius
				);

				glm::vec3 p2 = center + glm::vec3(
					sin(phi2) * cos(theta) * radius,
					cos(phi2) * radius,
					sin(phi2) * sin(theta) * radius
				);

				DrawLine(p1, p2, color);
			}
		}
	}

	void RenderScene(Scene& scene, bool renderUI) {
		glDisable(GL_DITHER);
		Vec2 framebufferResolution = Vec2(Window::Get()->viewportWidth, Window::Get()->viewportHeight);
		glClearColor(0, 0, 0, 1);

		ClearRenderTargets();
		UpdateSSBOS();
		if (framebufferResolution.x != lastResolution.x ||
			framebufferResolution.y != lastResolution.y) {
			//GetFrameBuffer("GBuffer")->Resize(framebufferResolution.x, framebufferResolution.y);
			lastResolution = framebufferResolution;
		}
		ShadowPass(scene);
		//SkyboxPass();
		GBufferPass(scene);
		ShadingPass(scene);
		DebugPass(scene);

		BlitFrameBuffer(&g_frameBuffers["GBuffer"], &g_frameBuffers["FinalImage"], "Lighting", "Color", GL_COLOR_BUFFER_BIT, GL_LINEAR);
		BlitToDefaultFrameBuffer(&g_frameBuffers["FinalImage"], "Color", GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//OutlinePass(scene);
	}

	OpenGLSSBO* GetSSBO(const std::string& name) {
		auto it = g_ssbos.find(name);
		return (it != g_ssbos.end()) ? &it->second : nullptr;
	}

	Shader* GetShader(const std::string& name) {
		auto it = g_shaders.find(name);
		return (it != g_shaders.end()) ? &it->second : nullptr;
	}

	FrameBuffer* GetFrameBuffer(const std::string& name) {
		auto it = g_frameBuffers.find(name);
		return (it != g_frameBuffers.end()) ? &it->second : nullptr;
	}

	void GBufferPass(Scene& scene) {
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		gBuffer->Bind();
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldSpacePosition", "MousePick" });
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			Shader* gBufferShader = GetShader("gBuffer");
			gBufferShader->Bind();
			gBufferShader->SetMat4("projection", m_Projection);
			gBufferShader->SetMat4("view", m_View);
			gBufferShader->SetMat4("lightProjectionMatrix", orthgonalProjection * lightView);

			// Models
			{
				//auto view = scene.Reg().view<TransformComponent, MeshRendererComponent>();
				//for (auto e : view) {
				//	auto [transform, mesh] = view.get<TransformComponent, MeshRendererComponent>(e);
				//	if (mesh._Model && scene.GetEntityByID((int)e)->IsActive()) {
				//		for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
				//			gBufferShader->SetInt("materialCount", m->GetMaterial()->id);
				//			gBufferShader->SetInt("aEntityID", (int)e + 1);
				//			gBufferShader->SetMat4("modelMatrix", transform.GetLocalMatrix());
				//			glActiveTexture(GL_TEXTURE5);
				//			glBindTexture(GL_TEXTURE_2D, depthMap);
				//			m->Draw(gBufferShader);
				//		}
				//	}
				//}
			}

			// Skinned Models
			{
				auto view = scene.Reg().view<TransformComponent, SkinnedMeshRendererComponent>();
				for (auto e : view) {
					auto [transform, mesh] = view.get<TransformComponent, SkinnedMeshRendererComponent>(e);
					if (mesh.Model && scene.GetEntityByID((int)e)->IsActive()) {
						for (SkinnedMesh m : mesh.Model->GetMeshes()) {
							gBufferShader->SetInt("aEntityID", (int)e + 1);
							gBufferShader->SetMat4("modelMatrix", transform.GetLocalMatrix());
							m.Draw(gBufferShader);
						}
					}
				}
			}

			// Quake BSPs
			//auto quakeView = scene.Reg().view<TransformComponent, BSPBrushComponent, VisibilityComponent>();
			//for (auto e : quakeView)
			//{
			//	auto [transform, model, visibility] = quakeView.get<TransformComponent, BSPBrushComponent, VisibilityComponent>(e);
			//
			//	if (model.IsTransparent || !visibility.Visible)
			//		continue;
			//}
			//
			//// Sprites
			//auto spriteView = scene.Reg().view<TransformComponent, SpriteComponent, VisibilityComponent>();
			//for (auto e : spriteView)
			//{
			//	auto [transform, sprite, visibility] = spriteView.get<TransformComponent, SpriteComponent, VisibilityComponent>(e);
			//
			//	if (!sprite.SpriteMesh || !visibility.Visible)
			//		continue;
			//
			//	auto finalQuadTransform = transform.GetGlobalMatrix();
			//	if (sprite.Billboard) {
			//		if (sprite.PositionFacing) {
			//
			//			const Matrix4& invView = glm::inverse(m_View);
			//			const Vec3& cameraPosition = Vec3(invView[3][0], invView[3][1], invView[3][2]);
			//			const Vec3& spritePosition = Vec3(finalQuadTransform[3][0], finalQuadTransform[3][1], finalQuadTransform[3][2]);
			//			const Vec3& direction = cameraPosition - spritePosition;
			//			finalQuadTransform = glm::inverse(glm::lookAt(Vec3(), direction, Vec3(0, 1, 0)));
			//		}
			//		else {
			//			finalQuadTransform = glm::inverse(m_View);
			//		}
			//
			//		if (sprite.LockYRotation) {
			//			// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
			//			finalQuadTransform[1] = Vec4(0, 1, 0, 0);
			//			finalQuadTransform[2] = Vec4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
			//			finalQuadTransform = finalQuadTransform;
			//		}
			//
			//		finalQuadTransform[3] = Vec4(Vec3(transform.GetGlobalMatrix()[3]), 1.0f);
			//
			//		// Scale
			//		finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
			//	}
			//
			//	Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (int32_t)e);
			//}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool cubemapIsLoaded = false;
	void ShadingPass(Scene& scene) {
		// ZoneScoped
		Shader* shader = GetShader("deferred");
		g_frameBuffers["GBuffer"].Bind();
		glViewport(0, 0, g_frameBuffers["GBuffer"].GetWidth(), g_frameBuffers["GBuffer"].GetHeight());
		g_frameBuffers["GBuffer"].DrawBuffer("Lighting");
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			shader->Bind();
			shader->SetMat4("projection", m_Projection);
			shader->SetMat4("view", m_View);
			shader->SetVec3("CamPosition", m_CamPos);
			shader->SetFloat("time", Engine::GetTime());
			shader->SetMat4("lightProjectionView", orthgonalProjection * lightView);
			shader->SetInt("lightCount", lightCount);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["GBuffer"].GetDepthAttachmentHandle());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["GBuffer"].GetColorAttachmentHandleByName("BaseColor"));
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["GBuffer"].GetColorAttachmentHandleByName("Normal"));
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["GBuffer"].GetColorAttachmentHandleByName("RMA"));
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["GBuffer"].GetColorAttachmentHandleByName("WorldSpacePosition"));
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, depthMap);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			Renderer::DrawQuad();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowPass(Scene& scene) {
		glDepthMask(true);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		Shader* shadowMapShader = GetShader("ShadowMap");
		shadowMapShader->Bind();
		shadowMapShader->SetMat4("lightSpaceMatrix", orthgonalProjection * lightView);

		// Models
		//auto view = scene.Reg().view<TransformComponent, MeshRendererComponent>();
		//for (auto e : view) {
		//	auto [transform, mesh] = view.get<TransformComponent, MeshRendererComponent>(e);
		//	if (mesh._Model && scene.GetEntityByID((int64_t)e)->IsActive()) {
		//		for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
		//			shadowMapShader->SetMat4("modelMatrix", transform.GetLocalMatrix());
		//			m->Draw(shadowMapShader, false);
		//		}
		//	}
		//}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SkyboxPass() {
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		Shader* shader = GetShader("skybox");

		gBuffer->Bind();
		gBuffer->DrawBuffers({ "Lighting", "WorldSpacePosition" });
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());

		Transform2 skyboxTransform;
		skyboxTransform.position = Engine::GetCurrentScene()->GetCurrentCamera()->GetPosition();
		skyboxTransform.scale = glm::vec3(100.0f * 0.99f);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		shader->Bind();
		shader->SetMat4("projection", m_Projection);
		shader->SetMat4("view", m_View);
		shader->SetMat4("modelMatrix", skyboxTransform.to_mat4());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapTextures["NightSky"].GetID());
		Renderer::DrawCube();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// CleanUp
		glDepthMask(GL_TRUE);
	}

	OpenGLDebugMesh g_debugMeshPoints;
	OpenGLDebugMesh g_debugMeshLines;

	void DebugPass(Scene& scene) {
		Shader* shader = GetShader("DebugVertex");
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");

		gBuffer->Bind();
		gBuffer->DrawBuffer("Lighting");

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glPointSize(8.0f);

		shader->Bind();
		UpdateDebugMesh();
		shader->SetMat4("u_projectionView", m_Projection * m_View);

		if (g_debugMeshLines.GetVertexCount() > 0) {
			glBindVertexArray(g_debugMeshLines.GetVAO());
			glDrawArrays(GL_LINES, 0, g_debugMeshLines.GetVertexCount());
		}
		if (g_debugMeshPoints.GetVertexCount() > 0) {
			glBindVertexArray(g_debugMeshPoints.GetVAO());
			glDrawArrays(GL_POINTS, 0, g_debugMeshPoints.GetVertexCount());
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OutlinePass(Scene& scene) {
		if (Engine::IsPlayMode()) return;
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		Shader* shader = GetShader("Outline");

		g_frameBuffers["FinalImage"].Bind();
		glViewport(0, 0, g_frameBuffers["FinalImage"].GetWidth(), g_frameBuffers["FinalImage"].GetHeight());
		shader->Bind();

		/*if (m_hoveredEntityID != 0) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 1);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			shader->SetMat4("projection", m_Projection);
			shader->SetMat4("view", m_View);
			shader->SetFloat("viewportWidth", Window::Get()->viewportWidth);
			shader->SetFloat("viewportHeight", Window::Get()->viewportHeight);

			Entity entity = { (entt::entity)(m_hoveredEntityID - 1), Engine::GetCurrentScene().get() };
			TransformComponent& transform = entity.GetComponent<TransformComponent>();
			MeshRendererComponent& mesh = entity.GetComponent<MeshRendererComponent>();
			VisibilityComponent& visiblity = entity.GetComponent<VisibilityComponent>();
			if (mesh._Model && visiblity.Visible) {
				for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
					shader->SetMat4("modelMatrix", transform.GetLocalMatrix());
					m->Draw(shader);
				}
			}

			// Render outline
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glEnable(GL_STENCIL_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			shader->SetVec3("Color", glm::vec3(YELLOW.x, YELLOW.y, YELLOW.z) - glm::vec3(0.5f));

			if (mesh._Model && visiblity.Visible) {
				for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
					shader->SetMat4("modelMatrix", transform.GetLocalMatrix());
					m->Draw(shader);
				}
			}
		}
		if (m_selectedEntityID != 0) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xff);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 1);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			shader->SetMat4("projection", m_Projection);
			shader->SetMat4("view", m_View);

			Entity entity = { (entt::entity)m_selectedEntityID, Engine::GetCurrentScene().get() };
			TransformComponent transform = entity.GetComponent<TransformComponent>();
			MeshRendererComponent mesh = entity.GetComponent<MeshRendererComponent>();
			VisibilityComponent visiblity = entity.GetComponent<VisibilityComponent>();
			if (mesh._Model && visiblity.Visible) {
				for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
					shader->SetMat4("modelMatrix", transform.GetLocalMatrix());
					m->Draw(shader);
				}
			}

			// Render outline
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glEnable(GL_STENCIL_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_DEPTH_TEST);
			shader->SetVec3("Color", YELLOW);

			if (mesh._Model && visiblity.Visible) {
				for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
					shader->SetMat4("modelMatrix", transform.GetLocalMatrix());
					m->Draw(shader);
				}
			}
		}*/

		// Cleanup
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (scene.selectedType == _CAMERA) {
			auto camView = scene.Reg().view<TransformComponent, CameraComponent>();
			for (auto e : camView) {
				auto [transform, camera] = scene.Reg().get<TransformComponent, CameraComponent>(e);

				const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
				const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
				Matrix4 clampedPerspective = glm::perspective(glm::radians(camera.camera->m_fieldOfView), Window::Get()->viewportWidth / Window::Get()->viewportHeight, camera.camera->m_nearPlane, camera.camera->m_farPlane);

				float fov = glm::radians(camera.camera->m_fieldOfView);
				float aspect = Engine::GetCurrentWindow()->viewportWidth / Engine::GetCurrentWindow()->viewportHeight;
				float nearDist = camera.camera->m_nearPlane;
				float farDist = camera.camera->m_farPlane;

				Vec3 pos = transform.GetGlobalPosition();
				Vec3 forward = camera.camera->GetForward();
				Vec3 right = camera.camera->GetRight();
				Vec3 up = camera.camera->GetUp();

				float height = tan(fov / 2.0f) * farDist;
				float width = height * aspect;

				Vec3 centerFar = pos + forward * farDist;
				Vec3 topLeft = centerFar + (up * height) - (right * width);
				Vec3 topRight = centerFar + (up * height) + (right * width);
				Vec3 bottomLeft = centerFar - (up * height) - (right * width);
				Vec3 bottomRight = centerFar - (up * height) + (right * width);

				DrawLine(pos, topLeft * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(pos, topRight * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(pos, bottomLeft * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(pos, bottomRight * glm::vec3(-1), { 1, 1, 1 });

				DrawLine(topLeft * glm::vec3(-1), topRight * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(topRight * glm::vec3(-1), bottomRight * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(bottomRight * glm::vec3(-1), bottomLeft * glm::vec3(-1), { 1, 1, 1 });
				DrawLine(bottomLeft * glm::vec3(-1), topLeft * glm::vec3(-1), { 1, 1, 1 });
			}
		}
		if (scene.selectedType == _LIGHT) {
			auto view = Engine::GetCurrentScene()->Reg().view<TransformComponent, LightComponent>();
			for (auto e : view) {
				auto [transform, lightComponent] = view.get<TransformComponent, LightComponent>(e);
				glm::vec3 position = transform.GetGlobalPosition();
				float radius = lightComponent.Radius;
				//DebugDrawWireSphere(position, radius * 0.5f, glm::vec3(1, 1, 0));
			}
		}
	}

	void UpdateDebugMesh() {
		g_debugMeshPoints.UpdateVertexData(g_points);
		g_debugMeshLines.UpdateVertexData(g_lines);

		g_points.clear();
		g_lines.clear();
	}

	void DrawLine(Vec3 begin, Vec3 end, Vec3 color, bool obeyDepth) {
		DebugVertex v0 = DebugVertex(begin, color, glm::ivec2(0, 0));
		DebugVertex v1 = DebugVertex(end, color, glm::ivec2(0, 0));
		if (obeyDepth) {

		}
		else {
			g_lines.push_back(v0);
			g_lines.push_back(v1);
		}
	}

	void DrawAABB(const AABB& aabb, const glm::vec3& color) {
		glm::vec3 FrontTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
		glm::vec3 FrontTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
		glm::vec3 FrontBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
		glm::vec3 FrontBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
		glm::vec3 BackTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
		glm::vec3 BackTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
		glm::vec3 BackBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
		glm::vec3 BackBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
		DrawLine(FrontTopLeft, FrontTopRight, color);
		DrawLine(FrontBottomLeft, FrontBottomRight, color);
		DrawLine(BackTopLeft, BackTopRight, color);
		DrawLine(BackBottomLeft, BackBottomRight, color);
		DrawLine(FrontTopLeft, FrontBottomLeft, color);
		DrawLine(FrontTopRight, FrontBottomRight, color);
		DrawLine(BackTopLeft, BackBottomLeft, color);
		DrawLine(BackTopRight, BackBottomRight, color);
		DrawLine(FrontTopLeft, BackTopLeft, color);
		DrawLine(FrontTopRight, BackTopRight, color);
		DrawLine(FrontBottomLeft, BackBottomLeft, color);
		DrawLine(FrontBottomRight, BackBottomRight, color);
	}

	void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform) {
		glm::vec4 FrontTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
		glm::vec4 FrontTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
		glm::vec4 FrontBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
		glm::vec4 FrontBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
		glm::vec4 BackTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
		glm::vec4 BackTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
		glm::vec4 BackBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
		glm::vec4 BackBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
		DrawLine(FrontTopLeft, FrontTopRight, color);
		DrawLine(FrontBottomLeft, FrontBottomRight, color);
		DrawLine(BackTopLeft, BackTopRight, color);
		DrawLine(BackBottomLeft, BackBottomRight, color);
		DrawLine(FrontTopLeft, FrontBottomLeft, color);
		DrawLine(FrontTopRight, FrontBottomRight, color);
		DrawLine(BackTopLeft, BackBottomLeft, color);
		DrawLine(BackTopRight, BackBottomRight, color);
		DrawLine(FrontTopLeft, BackTopLeft, color);
		DrawLine(FrontTopRight, BackTopRight, color);
		DrawLine(FrontBottomLeft, BackBottomLeft, color);
		DrawLine(FrontBottomRight, BackBottomRight, color);
	}

	void ClearRenderTargets() {
		/* ^^^^^^ Clear All FrameBuffers ^^^^^^ */
		{
			FrameBuffer* gBuffer = &g_frameBuffers["GBuffer"];
			// GBuffer
			glDepthMask(GL_TRUE);
			gBuffer->Bind();
			gBuffer->ClearAttachment("BaseColor", 0, 0, 0, 0);
			gBuffer->ClearAttachment("Normal", 0, 0, 0, 0);
			gBuffer->ClearAttachment("RMA", 0, 0, 0, 0);
			gBuffer->ClearAttachment("WorldSpacePosition", 0, 0);
			gBuffer->ClearDepthAttachment();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}