#include "SceneRenderer.h"
#include "Shader.h"
#include "Renderer/Types/ShadowMap.h"
#include "Renderer/Types/SSBO.hpp"
#include "Renderer/Types/GL_debug_mesh.hpp"

#include "Engine.h"
#include "RenderCommand.h"
#include "Scene/Components.h"
#include "Scene/Components/ParentComponent.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Input/Input.h"
#include "AssetManagment/Config.h"
#include "Util/Util.h"

namespace Faint::SceneRenderer {
	std::unordered_map<std::string, Shader> g_shaders;
	std::unordered_map<std::string, FrameBuffer> g_frameBuffers;
	std::unordered_map<std::string, CubemapTexture> g_cubemapTextures;
	std::unordered_map<std::string, OpenGLSSBO> g_ssbos;
	int lightCount;

	glm::vec3 g_lightDir = glm::normalize(glm::vec3(-2.0f, 4.0f, -1.0f));
	glm::vec3 g_lightPos = -g_lightDir * 10.0f;

	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
	glm::mat4 lightView = glm::lookAt(g_lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 g_lightSpaceMatrix;

	ShadowMap* m_ShadowMap;
	Matrix4 m_Projection, m_View;
	Vec3 m_CamPos;

	Vec2 lastResolution = Vec2();
	
	void ClearRenderTargets();

	void Init() {

		mINI::INIFile file = Config::Begin("config/layout.ini", true);
		std::string viewportStr = Config::currentINI["Viewport"]["Size"];
		std::stringstream ss(viewportStr);
		Vec2 defaultResolution = Vec2();
		char comma;
		ss >> defaultResolution.x >> comma >> defaultResolution.y;
		lastResolution = defaultResolution;

		g_frameBuffers["GBuffer"] = FrameBuffer("GBuffer", 1920, 1080);
		g_frameBuffers["GBuffer"].CreateAttachment("BaseColor", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Normal", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("RMA", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Lighting", GL_RGB16F);
		g_frameBuffers["GBuffer"].CreateAttachment("WorldSpacePosition", GL_RGBA32F);
		g_frameBuffers["GBuffer"].CreateAttachment("MousePick", GL_R32I);
		g_frameBuffers["GBuffer"].CreateDepthAttachment(GL_DEPTH_COMPONENT32F);

		//g_frameBuffers["Outline"] = FrameBuffer("Outline", defaultResolution.x, defaultResolution.y);
		//g_frameBuffers["Outline"].CreateAttachment("Mask", GL_R8);
		//g_frameBuffers["Outline"].CreateAttachment("Result", GL_R8);
		//g_frameBuffers["Outline"].CreateAttachment("Color", GL_RGB16F);

		g_frameBuffers["FinalImage"] = FrameBuffer("FinalImage", Config::finalImageResolution.x, Config::finalImageResolution.y);
		g_frameBuffers["FinalImage"].CreateAttachment("Color", GL_RGB16F);

		m_ShadowMap = new ShadowMap(2048, 2048);

		// Generate debug meshes
		std::vector<Vertex> lineVertices{
			{ Vec3(0, 0, 0), Vec3(0, 0, 0), Vec2(0, 0) },
			{ Vec3(1, 1, 1), Vec3(0, 0, 0), Vec2(0, 0) }
		};
		std::vector<uint32_t> lineIndices{ 0, 1 };

		g_ssbos["Lights"] = OpenGLSSBO(sizeof(Light) * MAX_LIGHTS, GL_DYNAMIC_STORAGE_BIT);

		LoadShaders();
	}

	void LoadShaders() {
		g_shaders["gBuffer"] = Shader({ "data/shaders/gBuffer.vert", "data/shaders/gBuffer.frag" });
		g_shaders["deferred"] = Shader({ "data/shaders/deferred.vert", "data/shaders/deferred.frag" });
		g_shaders["shadowMap"] = Shader({ "data/shaders/shadowMap.vert", "data/shaders/shadowMap.frag" });
		g_shaders["skybox"] = Shader({ "data/shaders/skybox.vert", "data/shaders/skybox.frag" });
		g_shaders["DebugVertex"] = Shader({ "data/shaders/debug_vertex.vert", "data/shaders/debug_vertex.frag" });
	}

	void Cleanup() {
	}

	void Update(const Time time) {
		if (Engine::IsPlayMode()) {
			Renderer2D::Update();
		}
	}

	void UpdateSSBOS() {
		std::vector<Light> lights{};
		auto view = Engine::GetCurrentScene()->Reg().view<TransformComponent, LightComponent>();
		for (auto e : view) {
			auto [transform, lightComponent] = view.get<TransformComponent, LightComponent>(e);
			Light light;
			light.posX = transform.GetGlobalPosition().x;
			light.posY = transform.GetGlobalPosition().y;
			light.posZ = transform.GetGlobalPosition().z;
			light.colorR = lightComponent.Color.r;
			light.colorG = lightComponent.Color.g;
			light.colorB = lightComponent.Color.b;
			light.radius = lightComponent.Radius;
			light.strength = lightComponent.Strength;
			light.type = (int)lightComponent.Type;
			lights.push_back(light);
		}
		if (!lights.empty()) {
			g_ssbos["Lights"].Update(lights.size() * sizeof(Light), (void*)&lights[0]);
			g_ssbos["Lights"].Bind(1);
			lightCount = (int)lights.size();
		}
	}

	void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos) {
		m_Projection = projection;
		m_View = view;
		m_CamPos = camPos;
	}

	void RenderScene(Scene& scene, bool renderUI) {
		glDisable(GL_DITHER);
		Vec2 framebufferResolution = Vec2(Window::Get()->viewportWidth, Window::Get()->viewportHeight);

		ClearRenderTargets();
		UpdateSSBOS();
		ShadowPass(scene);
		if (framebufferResolution != lastResolution) {
			GetFrameBuffer("GBuffer")->Resize(framebufferResolution.x, framebufferResolution.y);
			lastResolution = framebufferResolution;
		}
		SkyboxPass();
		GBufferPass(scene);
		ShadingPass(scene);
		DebugPass(scene);
		OutlinePass(scene);
		
		BlitFrameBuffer(&g_frameBuffers["GBuffer"], &g_frameBuffers["FinalImage"], "Lighting", "Color", GL_COLOR_BUFFER_BIT, GL_LINEAR);
		BlitToDefaultFrameBuffer(&g_frameBuffers["FinalImage"], "Color", GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
		gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldSpacePosition", "MousePick" });
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		{
			// Init
			RenderCommand::Enable(RendererEnum::DEPTH_TEST);
			RenderCommand::Enable(RendererEnum::FACE_CULL);
			glCullFace(GL_BACK);

			Shader* gBufferShader = GetShader("gBuffer");
			gBufferShader->Bind();
			gBufferShader->SetMat4("projection", m_Projection);
			gBufferShader->SetMat4("view", m_View);

			// Models
			{
				auto view = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
				for (auto e : view) {
					auto [transform, mesh, visiblity] = view.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);

					//gBufferShader.SetMat4("modelMatrix", transform.GetGlobalMatrix());
					//gBufferShader.SetInt("aEntityID", (int)e + 1);
					if (mesh._Model && visiblity.Visible) {
						for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
							Renderer::SubmitMesh(m, transform.GetGlobalMatrix(), (uint32_t)e);
						}
					}
				}
				Renderer::Flush(gBufferShader, false);

				RenderCommand::Enable(RendererEnum::DEPTH_TEST);
				glDepthMask(GL_FALSE);
				Renderer::Flush(gBufferShader, true);
				glDepthMask(GL_TRUE);
			}

			// Quake BSPs
			auto quakeView = scene.Reg().view<TransformComponent, BSPBrushComponent, VisibilityComponent>();
			for (auto e : quakeView)
			{
				auto [transform, model, visibility] = quakeView.get<TransformComponent, BSPBrushComponent, VisibilityComponent>(e);

				if (model.IsTransparent || !visibility.Visible)
					continue;
			}

			// Sprites
			auto spriteView = scene.Reg().view<TransformComponent, SpriteComponent, VisibilityComponent>();
			for (auto e : spriteView)
			{
				auto [transform, sprite, visibility] = spriteView.get<TransformComponent, SpriteComponent, VisibilityComponent>(e);

				if (!sprite.SpriteMesh || !visibility.Visible)
					continue;

				auto finalQuadTransform = transform.GetGlobalMatrix();
				if (sprite.Billboard) {
					if (sprite.PositionFacing) {

						const Matrix4& invView = glm::inverse(m_View);
						const Vec3& cameraPosition = Vec3(invView[3][0], invView[3][1], invView[3][2]);
						const Vec3& spritePosition = Vec3(finalQuadTransform[3][0], finalQuadTransform[3][1], finalQuadTransform[3][2]);
						const Vec3& direction = cameraPosition - spritePosition;
						finalQuadTransform = glm::inverse(glm::lookAt(Vec3(), direction, Vec3(0, 1, 0)));
					}
					else {
						finalQuadTransform = glm::inverse(m_View);
					}

					if (sprite.LockYRotation) {
						// This locks the pitch rotation on the billboard, useful for trees, lamps, etc.
						finalQuadTransform[1] = Vec4(0, 1, 0, 0);
						finalQuadTransform[2] = Vec4(finalQuadTransform[2][0], 0, finalQuadTransform[2][2], 0);
						finalQuadTransform = finalQuadTransform;
					}

					finalQuadTransform[3] = Vec4(Vec3(transform.GetGlobalMatrix()[3]), 1.0f);

					// Scale
					finalQuadTransform = glm::scale(finalQuadTransform, transform.GetGlobalScale());
				}

				Renderer::SubmitMesh(sprite.SpriteMesh, finalQuadTransform, (int32_t)e);
			}
			Renderer::Flush(gBufferShader, false);

			auto fontView = scene.Reg().view<TransformComponent, TextComponent>();
			for (auto e : fontView) {

				auto [transform, text] = fontView.get<TransformComponent, TextComponent>(e);

				Renderer2D::DrawString(text.TextString, transform.GetGlobalMatrix(), text, (int)e);
			}
		}
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool cubemapIsLoaded = false;
	void ShadingPass(Scene& scene) {
		// ZoneScoped
		Shader* shader = GetShader("deferred");
		g_frameBuffers["GBuffer"].Bind();
		g_frameBuffers["GBuffer"].DrawBuffer("Lighting");
		glViewport(0, 0, g_frameBuffers["GBuffer"].GetWidth(), g_frameBuffers["GBuffer"].GetHeight());
		{
			shader->Bind();
			shader->SetMat4("projection", m_Projection);
			shader->SetMat4("view", m_View);
			shader->SetVec3("CamPosition", m_CamPos);
			shader->SetFloat("time", Engine::GetTime());
			shader->SetMat4("lightProjectionView", g_lightSpaceMatrix);
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
			glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			Renderer::DrawQuad();
		}
	}

	void ShadowPass(Scene& scene) {
		g_lightSpaceMatrix = lightProjection * lightView;
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ShadowMap->GetID());
			Shader* shadowMapShader = GetShader("shadowMap");
			shadowMapShader->Bind();
			shadowMapShader->SetMat4("projectionView", g_lightSpaceMatrix);

			auto view = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
			for (auto e : view) {
				auto [transform, mesh, visiblity] = view.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
				shadowMapShader->SetMat4("modelMatrix", transform.GetGlobalMatrix());
				if (mesh._Model && visiblity.Visible) {
					for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
						m->Draw(shadowMapShader, false);
					}
				}
			}
		}
	}

	void SkyboxPass() {
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		Shader* shader = GetShader("skybox");

		gBuffer->Bind();
		gBuffer->DrawBuffer("BaseColor");
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());

		Transform2 skyboxTransform;
		skyboxTransform.position = Engine::GetCurrentScene()->GetCurrentCamera()->GetPosition();
		skyboxTransform.scale = glm::vec3(100.0f * 0.99f);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		Ref<Environment> env = Engine::GetCurrentScene()->GetEnvironment();

		bool hasSky = false;
		auto skyView = Engine::GetCurrentScene()->Reg().view<SkyComponent>();
		for (auto l : skyView) {
			SkyComponent& sky = skyView.get<SkyComponent>(l);

			if (!cubemapIsLoaded) {
				g_cubemapTextures["NightSky"].Load("data/textures/NightSky");
				g_cubemapTextures["NightSky"].Bake();
				cubemapIsLoaded = true;
			}
			hasSky = true;
		}

		shader->Bind();
		shader->SetMat4("projection", Engine::GetCurrentScene()->GetCurrentCamera()->GetProjectionMatrix());
		shader->SetMat4("view", Engine::GetCurrentScene()->GetCurrentCamera()->GetViewMatrix());
		if (hasSky) {
			shader->SetMat4("modelMatrix", skyboxTransform.to_mat4());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapTextures["NightSky"].GetID());
			Renderer::DrawCube();
		}

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
	}

	void OutlinePass(Scene& scene) {
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		FrameBuffer* outlineFBO = GetFrameBuffer("Outline");

		//{
		//	FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		//	FrameBuffer* outlineBuffer = GetFrameBuffer("Outline");
		//	//Shader maskShader = ShaderManager::GetShader("outlineMask");
		//	Shader shader = ShaderManager::GetShader("outline");
		//	
		//	outlineBuffer->BindDepthAttachmentFrom(*gBuffer);
		//	outlineBuffer->Bind();
		//	//outlineBuffer->ClearAttachmentI("Mask", 0);
		//	//outlineBuffer->ClearAttachmentI("Result", 0);
		//	outlineBuffer->ClearAttachment("Color", 0, 0, 0);
		//	glDisable(GL_DEPTH_TEST);
		//	glDisable(GL_BLEND);
		//	shader.Bind();
		//
		//	glViewport(0, 0, g_frameBuffers["Outline"].GetWidth(), g_frameBuffers["Outline"].GetHeight());
		//	outlineBuffer->DrawBuffer("Color");
		//	
		//	Renderer::DrawQuad();
		//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//}

		/*if (scene.selectedType == _CAMERA)
			{
				auto camView = scene.Reg().view<TransformComponent, CameraComponent>();
				for (auto e : camView)
				{
					auto [transform, camera] = scene.Reg().get<TransformComponent, CameraComponent>(e);
					shader.SetFloat("_Opacity", 0.4f);

					const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
					const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
					Matrix4 clampedPerspective = glm::perspective(glm::radians(camera.camera->m_fieldOfView), Window::Get()->viewportWidth / Window::Get()->viewportHeight, camera.camera->m_nearPlane, camera.camera->m_farPlane);
					shader.SetVec4("Color", Vec4(1, 1, 1, 1));
					shader.SetMat4("view", glm::translate(scene.GetCurrentCamera()->GetViewMatrix(), Vec3(transform.GetGlobalMatrix()[3])) * rotationMatrix * glm::inverse(clampedPerspective));

					RenderCommand::DrawLines(0, 26);
				}
			}*/
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
			//gBuffer->ClearAttachmentI("MousePick", -1);
			gBuffer->ClearDepthAttachment();
		}
	}
}