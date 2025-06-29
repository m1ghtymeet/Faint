#include "SceneRenderer.h"
#include "Shader.h"
#include "Renderer/OpenGL/Types/ShadowMap.h"
#include "Renderer/OpenGL/Types/GL_debug_mesh.hpp"
#include "Renderer/OpenGL/Types/CubemapTexture.h"
#include "Renderer/OpenGL/Types/CubemapView.h"

#include "Engine.h"
#include "Scene/Components.h"
#include "Renderer.h"
#include "Input/Input.h"
#include "AssetManagment/Config.h"
#include "Util/Util.h"
#include "Scene/Entity.h"

#define SHADOW_MAP_SIZE 2048

namespace Faint::SceneRenderer {
	std::unordered_map<std::string, Shader> g_shaders;
	std::unordered_map<std::string, FrameBuffer> g_frameBuffers;
	std::unordered_map<std::string, CubemapTexture> g_cubemapTextures;
	std::unordered_map<std::string, OpenGLCubemapView> g_cubemapViews;
	std::unordered_map<std::string, OpenGLSSBO> g_ssbos;
	int lightCount, materialCount;

	glm::mat4 orthgonalProjection, lightView;

	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 20.5f);
	std::vector<glm::mat4> shadowTransforms;

	//ShadowMap* m_ShadowMap;
	Matrix4 m_Projection, m_View;
	Vec3 m_CamPos;
	Model* CameraModel;

	Vec2 lastResolution = Vec2();

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

		g_frameBuffers["ShadowMap"] = FrameBuffer("ShadowMap", SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		g_frameBuffers["ShadowMap"].CreateDepthAttachment(
			GL_DEPTH_COMPONENT32F,
			GL_NEAREST,
			GL_NEAREST,
			GL_CLAMP_TO_BORDER,
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
		);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		CameraModel = AssetManager::LoadModel("data/editor/models/Camera.obj", true);

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
		for (Entity* entity : Engine::GetCurrentScene()->GetAllEntities()) {
			TransformComponent* transform = entity->GetComponent<TransformComponent>();
			LightComponent* lightComponent = entity->GetComponent<LightComponent>();
			if (lightComponent) {
				Light light;
				light.posX = transform->GetLocalPosition().x; light.posY = transform->GetLocalPosition().y;
				light.posZ = transform->GetLocalPosition().z; light.colorR = lightComponent->Color.r;
				light.colorG = lightComponent->Color.g; light.colorB = lightComponent->Color.b;
				light.radius = lightComponent->Radius; light.strength = lightComponent->Strength;
				light.type = (int)lightComponent->Type;
				lights.push_back(light);
			}
		}
		if (!lights.empty()) {
			g_ssbos["Lights"].Update(lights.size() * sizeof(Light), (void*)&lights[0]);
			g_ssbos["Lights"].Bind(1);
			lightCount = (int)lights.size();
		}

		std::vector<MaterialData> materials{};
		auto matView = Engine::GetCurrentScene()->Reg().view<MeshRendererComponent>();
		for (auto e : matView) {
			MeshRendererComponent& meshComponent = matView.get<MeshRendererComponent>(e);
			for (Ref<Mesh> mesh : meshComponent.GetModel()->GetMeshes()) {
				materials.push_back(mesh->GetMaterial()->data);
				mesh->GetMaterial()->id++;
			}
		}
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
			float phi = glm::pi<float>() * float(i) / float(rings); // زاویه بین 0 تا pi
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
		//OutlinePass(scene);

		BlitFrameBuffer(&g_frameBuffers["GBuffer"], &g_frameBuffers["FinalImage"], "Lighting", "Color", GL_COLOR_BUFFER_BIT, GL_LINEAR);
		BlitToDefaultFrameBuffer(&g_frameBuffers["FinalImage"], "Color", GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

			for (Entity* e : Engine::GetCurrentScene()->GetAllEntities()) {
				TransformComponent* transform = e->GetComponent<TransformComponent>();
				MeshRendererComponent* mesh = e->GetComponent<MeshRendererComponent>();
				if (mesh && mesh->GetModel()) {
					for (Ref<Mesh> m : mesh->GetModel()->GetMeshes()) {
						gBufferShader->SetInt("materialCount", m->GetMaterial()->id);
						gBufferShader->SetInt("aEntityID", e->GetID());
						gBufferShader->SetMat4("modelMatrix", transform->GetLocalMatrix());
						m->Draw(gBufferShader);
					}
				}

				// Render Camera
				//CameraComponent* camera = e->GetComponent<CameraComponent>();
				//if (camera) {
				//	for (Ref<Mesh> m : CameraModel->GetMeshes()) {
				//		gBufferShader->SetInt("aEntityID", e->GetID());
				//		m->Draw(gBufferShader);
				//	}
				//}
			}


			// Skinned Models
			//{
			//	auto view = scene.Reg().view<TransformComponent, SkinnedMeshRendererComponent>();
			//	for (auto e : view) {
			//		auto [transform, mesh] = view.get<TransformComponent, SkinnedMeshRendererComponent>(e);
			//		bool visible = scene.GetEntityByID((int)e)->IsActive();
			//		if (mesh.Model && visible) {
			//			for (SkinnedMesh m : mesh.Model->GetMeshes()) {
			//				gBufferShader->SetInt("aEntityID", (int)e + 1);
			//				gBufferShader->SetMat4("modelMatrix", transform.GetLocalMatrix());
			//				m.Draw(gBufferShader);
			//			}
			//		}
			//	}
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
			glBindTexture(GL_TEXTURE_2D, g_frameBuffers["ShadowMap"].GetDepthAttachmentHandle());

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			Renderer::DrawQuad();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowPass(Scene& scene) {
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT); // Front face culling برای بهتر شدن shadow acne
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		g_frameBuffers["ShadowMap"].Bind();
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glClear(GL_DEPTH_BUFFER_BIT);

		// پیدا کردن اولین Directional Light
		glm::vec3 lightPosition = glm::vec3(20.0f, 20.0f, 20.0f);
		glm::vec3 lightDirection = glm::vec3(-0.5f, -0.5f, -0.5f);

		//for (Entity* entity : Engine::GetCurrentScene()->GetAllEntities()) {
		//	LightComponent* lightComp = entity->GetComponent<LightComponent>();
		//	TransformComponent* transform = entity->GetComponent<TransformComponent>();
		//
		//	if (lightComp /* && lightComp->Type == LightType::Directional*/) {
		//		lightPosition = transform->GetLocalPosition();
		//		// فرض: forward vector برای direction
		//		glm::mat4 rotMatrix = glm::mat4_cast(transform->GetLocalRotation());
		//		lightDirection = glm::vec3(rotMatrix * glm::vec4(0, 0, -1, 0));
		//		break;
		//	}
		//}

		// محاسبه Light Space Matrix بهبود یافته
		glm::vec3 lightTarget = lightPosition + lightDirection;
		lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0, 1, 0));

		// تنظیم Orthographic projection بر اساس صحنه
		float orthoSize = 35.0f; // می‌تونی این رو بر اساس scene bounds تنظیم کنی
		glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// Update global matrices
		orthgonalProjection = lightProjection;
		//::lightView = lightView; // اگر global هست

		Shader* shadowMapShader = GetShader("ShadowMap");
		shadowMapShader->Bind();
		shadowMapShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		// Render all shadow casters
		for (Entity* e : Engine::GetCurrentScene()->GetAllEntities()) {
			TransformComponent* transform = e->GetComponent<TransformComponent>();
			MeshRendererComponent* mesh = e->GetComponent<MeshRendererComponent>();

			if (mesh && mesh->GetModel()) {
				// چک کن اگه cast shadow داره
				for (Ref<Mesh> m : mesh->GetModel()->GetMeshes()) {
					shadowMapShader->SetMat4("modelMatrix", transform->GetLocalMatrix());
					m->Draw(shadowMapShader, false);
				}
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK); // برگرداندن به حالت عادی
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
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		shader->Bind();
		shader->SetMat4("projection", m_Projection);
		shader->SetMat4("view", m_View);
		shader->SetMat4("modelMatrix", skyboxTransform.to_mat4());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapTextures["NightSky"].GetID());
		Renderer::DrawCube();

		// Cleanup
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
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

		gBuffer->Bind();
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		shader->Bind();
		shader->SetMat4("u_projectionView", m_Projection * m_View);
		shader->SetFloat("viewportWidth", Window::Get()->viewportWidth);
		shader->SetFloat("viewportHeight", Window::Get()->viewportHeight);

		auto renderEntityOutline = [&](int entityID, const glm::vec3& color, float outlineWidth = 1.02f) {
			if (entityID == 0) return;

			Entity* entity = Engine::GetCurrentScene()->GetEntityByID(entityID);
			if (!entity || !entity->IsActive()) return;

			TransformComponent* transform = entity->GetComponent<TransformComponent>();
			MeshRendererComponent* mesh = entity->GetComponent<MeshRendererComponent>();
		
			if (!mesh || !mesh->GetModel()) return;

			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);

			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_DEPTH_TEST);

			for (Ref<Mesh> m : mesh->GetModel()->GetMeshes()) {
				shader->SetMat4("modelMatrix", transform->GetLocalMatrix());
				shader->SetFloat("outlineWidth", 1.0f);
				shader->SetVec3("Color", glm::vec3(0.0f));
				m->Draw(shader);
			}

			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00); // Don't write to stencil
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Write to color
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (Ref<Mesh> m : mesh->GetModel()->GetMeshes()) {
				shader->SetMat4("modelMatrix", transform->GetLocalMatrix());
				shader->SetFloat("outlineWidth", outlineWidth);
				shader->SetVec3("Color", color);
				m->Draw(shader);
			}

			glDisable(GL_STENCIL_TEST);
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		};

		if (m_hoveredEntityID != 0) {
			renderEntityOutline(m_hoveredEntityID, YELLOW * 0.7f, 1.015f);
		}

		if (m_selectedEntityID != 0 && m_selectedEntityID != m_hoveredEntityID) {
			renderEntityOutline(m_selectedEntityID, YELLOW, 1.025f);
		}

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

				DrawLine(pos, topLeft * glm::vec3(-1), {1, 1, 1});
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

	void OutlinePassEdgeDetection(Scene& scene) {

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