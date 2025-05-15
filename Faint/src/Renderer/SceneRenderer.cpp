#include "SceneRenderer.h"
#include "RenderCommand.h"
#include "ShadersManager.h"
#include "Renderer.h"
#include "Engine.h"
#include "Scene/Components.h"
#include "Scene/Components/ParentComponent.h"
#include "AssetManagment/Project.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Types/ShadowMap.h"
#include "Input/Input.h"
#include "Util/Util.h"

#include <complex>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

namespace Faint {
	namespace SceneRenderer {
		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			if (srcAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(GL_BACK);
				glm::vec4 srcRect;
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.z = (float)srcFrameBuffer->GetWidth();
				srcRect.w = (float)srcFrameBuffer->GetHeight();
				glm::vec4 dstRect;
				dstRect.x = 0;
				dstRect.y = 0;
				dstRect.z = Window::Get()->GetWidth();
				dstRect.w = Window::Get()->GetHeight();
				glBlitFramebuffer(0, 0, (GLint)srcFrameBuffer->GetWidth(), (GLint)srcFrameBuffer->GetHeight(), 0, 0, (GLint)Window::Get()->GetWidth(), (GLint)Window::Get()->GetHeight(), mask, filter);
			}
		}

		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, glm::vec4 srcRect, glm::vec4 dstRect, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			if (srcAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer((GLint)srcRect.x, (GLint)srcRect.y, (GLint)srcRect.z, (GLint)srcRect.w, (GLint)dstRect.x, (GLint)dstRect.y, (GLint)dstRect.z, (GLint)dstRect.w, mask, filter);
			}
		}

		void BlitFrameBuffer(FrameBuffer* srcFrameBuffer, FrameBuffer* dstFrameBuffer, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			GLint dstAttachmentSlot = dstFrameBuffer->GetColorAttachmentSlotByName(dstName);
			if (srcAttachmentSlot != GL_INVALID_VALUE && dstAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFrameBuffer->GetHandle());
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(dstAttachmentSlot);
				glm::vec4 srcRect;
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.z = (float)srcFrameBuffer->GetWidth();
				srcRect.w = (float)srcFrameBuffer->GetHeight();
				glm::vec4 dstRect;
				dstRect.x = 0;
				dstRect.y = 0;
				dstRect.z = (float)dstFrameBuffer->GetWidth();
				dstRect.w = (float)dstFrameBuffer->GetHeight();
				glBlitFramebuffer(0, 0, (GLint)srcFrameBuffer->GetWidth(), (GLint)srcFrameBuffer->GetHeight(), 0, 0, (GLint)dstFrameBuffer->GetWidth(), (GLint)dstFrameBuffer->GetHeight(), mask, filter);
			}
		}

		Mesh sphereMesh;
		float previousRadiusChange = 0;
		std::unordered_map<std::string, FrameBuffer> g_frameBuffers;
		std::unordered_map<std::string, CubemapTexture> g_cubemapTextures;
		ShadowMap* m_ShadowMap;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0, 2, 0), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix;
		Matrix4 m_Projection, m_View;
		Vec3 m_CamPos;

		Ref<Mesh> mLineMesh;
		Ref<BoxGizmo> m_BoxGizmo;

		struct DebugShape {
			Vec3 Position;
			Quat Rotation;
			Color LineColor;
			float Life;
			float Width;
			bool DepthTest = true;
			//Ref<Physics::PhysicShape> Shape;
		};

		struct DebugLine {
			Vec3 Start;
			Vec3 End;
			Color LineColor;
			float Life;
			float Width;
			bool DepthTest = true;
		};

		std::vector<DebugLine> m_DebugLines;
		std::vector<DebugShape> m_DebugShapes;
	}

	void SceneRenderer::Init() {

		const auto defaultResolution = Vec2(1280, 720);
		g_frameBuffers = std::unordered_map<std::string, FrameBuffer>();
		g_frameBuffers["GBuffer"] = FrameBuffer("GBuffer", defaultResolution.x, defaultResolution.y);
		g_frameBuffers["GBuffer"].CreateAttachment("BaseColor", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Normal", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("RMA", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Lighting", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("WorldSpacePosition", GL_RGBA32F);
		g_frameBuffers["GBuffer"].CreateAttachment("MousePick", GL_R32I);
		g_frameBuffers["GBuffer"].CreateDepthAttachment(GL_DEPTH_COMPONENT32F);
		
		g_frameBuffers["Outline"] = FrameBuffer("Outline", defaultResolution.x, defaultResolution.y);
		g_frameBuffers["Outline"].CreateAttachment("Mask", GL_R8);
		g_frameBuffers["Outline"].CreateAttachment("Result", GL_R8);
		
		g_frameBuffers["FinalImage"] = FrameBuffer("FinalImage", defaultResolution.x, defaultResolution.y);
		g_frameBuffers["FinalImage"].CreateAttachment("Color", GL_RGB16F);

		//m_GBuffer = CreateScope<Framebuffer2>(false, defaultResolution);
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT); // Depth
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0); // Albedo
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT1); // Normal
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGBA), GL_COLOR_ATTACHMENT2); // Material + unlit
		
		//auto entityTexture = CreateRef<Texture>(defaultResolution, GL_RED_INTEGER, GL_R32I, GL_INT);
		//entityTexture->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		//m_GBuffer->SetTexture(entityTexture, GL_COLOR_ATTACHMENT3); // Entity ID
		//
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RED, GL_R16F, GL_FLOAT), GL_COLOR_ATTACHMENT4); // Emissive
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RG, GL_RG16F, GL_FLOAT), GL_COLOR_ATTACHMENT5); // Velocity
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RG, GL_RG, GL_UNSIGNED_BYTE), GL_COLOR_ATTACHMENT6); // UV
		//m_GBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGBA, GL_RGBA32F, GL_UNSIGNED_BYTE), GL_COLOR_ATTACHMENT7); // World Space Position

		//m_ShadingBuffer = CreateScope<Framebuffer2>(true, defaultResolution);
		//m_ShadingBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB, GL_RGB16F, GL_FLOAT));
		//m_ShadingBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT); // Depth

		//m_OutlineBuffer = CreateScope<Framebuffer2>(false, defaultResolution);
		//m_OutlineBuffer->SetTexture(CreateRef<Texture>(defaultResolution, GL_RGB), GL_COLOR_ATTACHMENT0);

		//g_cubemapTextures["NightSky"].Load("res/Textures/NightSky");
		//g_cubemapTextures["NightSky"].Bake();
		//cubeMesh = new Mesh();
		//cubeMesh->AddSurface(Util::GenerateCubeVertices(), Util::GenerateCubeIndices());

		//m_ShadowMap = new ShadowMap(1024, 1024);

		// Generate debug meshes
		std::vector<Vertex> lineVertices
		{
			{ Vec3(0, 0, 0), Vec3(0, 0, 0), Vec2(0, 0) },
			{ Vec3(1, 1, 1), Vec3(0, 0, 0), Vec2(0, 0) }
		};

		std::vector<uint32_t> lineIndices
		{
			0, 1
		};

		// Debug shapes
		mLineMesh = CreateRef<Mesh>();
		mLineMesh->AddSurface(lineVertices, lineIndices);

		m_BoxGizmo = CreateRef<BoxGizmo>();
		m_BoxGizmo->CreateMesh();

		m_DebugLines = std::vector<DebugLine>();
		m_DebugShapes = std::vector<DebugShape>();
	}

	void SceneRenderer::Cleanup()
	{
	}

	void SceneRenderer::Update(const Time time) {

		// Delete debug shapes that are dead
		if (m_DebugLines.size() > 0)
		{
			std::erase_if(m_DebugLines, [](const DebugLine& line)
				{
					return line.Life < 0.0f;
				});

			for (auto& line : m_DebugLines) {
				line.Life -= time;
			}
		}

		if (m_DebugShapes.size() > 0)
		{
			std::erase_if(m_DebugShapes, [](const DebugShape& shape)
				{
					return shape.Life < 0.0f;
				});

			for (auto& shape : m_DebugShapes)
			{
				shape.Life -= time;
			}
		}

		if (Engine::IsPlayMode()) {
			Renderer2D::Update();
		}
	}

	void SceneRenderer::BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos) {
		m_Projection = projection;
		m_View = view;
		m_CamPos = camPos;
	}

	void SceneRenderer::RenderScene(Scene& scene, bool renderUI) {

		const Vec2 framebufferResolution = Vec2(Window::Get()->viewportWidth, Window::Get()->viewportHeight);

		/* ^^^^^^ Clear All FrameBuffers ^^^^^^ */
		{
			FrameBuffer* gBuffer = &g_frameBuffers["GBuffer"];
			// GBuffer
			glDepthMask(GL_TRUE);
			gBuffer->ClearAttachment("BaseColor", 0, 0, 0, 0);
			gBuffer->ClearAttachment("Normal", 0, 0, 0, 0);
			gBuffer->ClearAttachment("RMA", 0, 0, 0, 0);
			gBuffer->ClearAttachment("WorldSpacePosition", 0, 0);
			gBuffer->ClearDepthAttachment();
		}

		//ShadowPass(scene);
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMap->GetID());
			//glClear(GL_DEPTH_BUFFER_BIT);
			//glViewport(0, 0, 1280, 720);
			//
			//Shader* shadowMapShader = ShaderManager::GetShader("shadowMap");
			//
			//shadowMapShader->Use();
			//
			//lightSpaceMatrix = lightProjection * lightView;
			//shadowMapShader->SetMat4("projectionView", lightSpaceMatrix);
			//
			//auto view = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
			//for (auto e : view) {
			//	auto [transform, mesh, visiblity] = view.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
			//
			//	shadowMapShader->SetMat4("modelMatrix", transform.GetGlobalMatrix());
			//	if (mesh._Model && visiblity.Visible) {
			//		for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
			//			Renderer::SubmitMesh(m, transform.GetGlobalMatrix(), (uint32_t)e);
			//		}
			//	}
			//}
			//
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		if (renderUI) {
			g_frameBuffers["GBuffer"].Resize(framebufferResolution.x, framebufferResolution.y);
		}

		//SkyboxPass();
		GBufferPass(scene);
		
		// Save previous Matrix
		auto modelView = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
		for (auto e : modelView) {
			auto [transform, mesh, visibility] = modelView.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
			Entity entity = { (entt::entity)e, Engine::GetCurrentScene().get() };
			if (!entity.IsValid())
				continue;

			if (mesh._Model && visibility.Visible) {
				transform.PreviousMatrix = m_Projection * m_View * transform.GetGlobalMatrix();
			}
		}

		ShadingPass(scene);

		BlitFrameBuffer(&g_frameBuffers["GBuffer"], &g_frameBuffers["FinalImage"], "Lighting", "Color", GL_COLOR_BUFFER_BIT, GL_LINEAR);
		//BlitToDefaultFrameBuffer(&g_frameBuffers["FinalImage"], "Color", GL_COLOR_BUFFER_BIT, GL_NEAREST);

		if (renderUI) {
			DebugPass(scene);
		}

		//if (Engine::IsPlayMode()) {
		//	framebuffer.Bind();
		//	Renderer2D::Render();
		//	framebuffer.Unbind();
		//}

		//auto textView = scene.Reg().view<TransformComponent, TextBlitterComponent>();
		//for (auto e : textView) {
		//
		//	auto [transform, text] = textView.get<TransformComponent, TextBlitterComponent>(e);
		//
		//	Renderer2D::BlitText(text.Text, text.FontName, text.LocationX, text.LocationY, text.Scale);
		//}

		//if (!Engine::IsPlayMode())
		//{
		//	auto boxView = scene.Reg().view<TransformComponent, BoxColliderComponent>();
		//	for (auto e : boxView) {
		//
		//		auto [transform, box] = boxView.get<TransformComponent, BoxColliderComponent>(e);
		//
		//		//scene.m_sceneRenderer->DrawDebugShape(transform.GetLocalPosition(), transform.GetLocalRotation(), CreateRef<Physics::Box>(box.Size.x, box.Size.y, box.Size.z), Vec4(1, 0, 0, 1), 0, 1);
		//	}
		//	auto sphereView = scene.Reg().view<TransformComponent, SphereColliderComponent>();
		//	for (auto e : sphereView) {
		//
		//		auto [transform, sphere] = sphereView.get<TransformComponent, SphereColliderComponent>(e);
		//
		//		//scene.m_sceneRenderer->DrawDebugShape(transform.GetLocalPosition(), transform.GetLocalRotation(), CreateRef<Physics::Sphere>(sphere.Radius), Vec4(1, 0, 0, 1), 0, 1);
		//	}
		//}

		RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	}

	FrameBuffer* SceneRenderer::GetFrameBuffer(const std::string& name) {
		return &g_frameBuffers[name];
	}

	void SceneRenderer::DrawDebugLine(const Vec3& start, const Vec3& end, const Color& color, float life, float width) {

		DebugLine debugLine = {
			start,
			end,
			color,
			life,
			width,
			true
		};

		m_DebugLines.push_back(debugLine);
	}

	void SceneRenderer::DrawDebugShape(const Vec3& position, const Quat& rotation, PxShape* shape, const Color& color, float life, float width)
	{
		DebugShape debugShape{
			.Position = position,
			.Rotation = rotation,
			.LineColor = color,
			.Life = life,
			.Width = width,
			.DepthTest = true,
			//.Shape = 
		};

		m_DebugShapes.push_back(debugShape);
	}

	void SceneRenderer::GBufferPass(Scene& scene) {

		g_frameBuffers["GBuffer"].Bind();
		g_frameBuffers["GBuffer"].DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldSpacePosition", "MousePick" });
		glViewport(0, 0, g_frameBuffers["GBuffer"].GetWidth(), g_frameBuffers["GBuffer"].GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			// Init
			RenderCommand::Enable(RendererEnum::DEPTH_TEST);
			RenderCommand::Enable(RendererEnum::FACE_CULL);


			Shader gBufferShader = ShaderManager::GetShader("gBuffer");
			
			gBufferShader.Bind();
			gBufferShader.SetMat4("projection", m_Projection);
			gBufferShader.SetMat4("view", m_View);

			// Models
			{
				auto view = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
				for (auto e : view) {
					auto [transform, mesh, visiblity] = view.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
					
					gBufferShader.SetMat4("model", transform.GetGlobalMatrix());
					gBufferShader.SetInt("aEntityID", (int)e);
					if (mesh._Model && visiblity.Visible) {
						for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
							Renderer::SubmitMesh(m, transform.GetGlobalMatrix(), (uint32_t)e);
						}
					}
				}
				Renderer::Flush(&gBufferShader, false);

				RenderCommand::Enable(RendererEnum::DEPTH_TEST);
				glDepthMask(GL_FALSE);
				Renderer::Flush(&gBufferShader, true);
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
			Renderer::Flush(&gBufferShader, false);

			auto fontView = scene.Reg().view<TransformComponent, TextComponent>();
			for (auto e : fontView) {

				auto [transform, text] = fontView.get<TransformComponent, TextComponent>(e);

				Renderer2D::DrawString(text.TextString, transform.GetGlobalMatrix(), text, (int)e);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool cubemapIsLoaded = false;
	void SceneRenderer::ShadingPass(Scene& scene) {

		// ZoneScoped
		g_frameBuffers["GBuffer"].Bind();
		g_frameBuffers["GBuffer"].DrawBuffer("Lighting");
		glViewport(0, 0, g_frameBuffers["GBuffer"].GetWidth(), g_frameBuffers["GBuffer"].GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			Ref<Environment> env = scene.GetEnvironment();

			Shader shader = ShaderManager::GetShader("deferred");
			shader.Bind();
			shader.SetMat4("projection", m_Projection);
			shader.SetMat4("view", m_View);
			shader.SetVec3("CamPosition", m_CamPos);
			shader.SetFloat("time", Engine::GetTime());
			shader.SetMat4("lightProjectionView", lightSpaceMatrix);

			struct LightDistance {
				TransformComponent transform;
				LightComponent light;
				float distance;
			};
			std::vector<LightDistance> lightDistances;

			auto view = scene.Reg().view<TransformComponent, LightComponent, ParentComponent, VisibilityComponent>();

			lightDistances.reserve(view.size_hint());

			const Vec3 camPosition = scene.GetCurrentCamera()->GetPosition();
			for (auto l : view)
			{
				auto [transform, light, parent, visibility] = view.get<TransformComponent, LightComponent, ParentComponent, VisibilityComponent>(l);

				if (visibility.Visible) {
					if (light.Type == Directional) {
						if (light.SyncDirectionWithSky) {
							// TODO: ProceduralSkybox
							if (light.SyncDirectionWithSky) {
								light.Direction = env->proceduralSkybox->GetSunDirection();
							}
							else {
								light.Direction = transform.GetGlobalRotation() * Vec3(0, 0, 1);
							}

						}
						else {
							light.Direction = transform.GetGlobalRotation() * Vec3(0, 0, 1);
						}
					}

					Vec3 lightPosition = transform.GetGlobalPosition();
					float distanceFromCam = glm::length(camPosition - lightPosition);

					lightDistances.push_back({ transform, light, distanceFromCam });
				}
			}

			std::sort(lightDistances.begin(), lightDistances.end(),
				[](const LightDistance& a, const LightDistance& b)
				{
					return a.distance < b.distance;
				}
			);

			for (const auto& l : lightDistances)
				Renderer::RegisterLight(l.transform, l.light);

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
			//glActiveTexture(GL_TEXTURE5);
			//glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());

			glDisable(GL_DEPTH_TEST);
			Renderer::DrawQuad();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SceneRenderer::ShadowPass(Scene& scene)
	{
		RenderCommand::Enable(RendererEnum::DEPTH_TEST);

		Shader shader = ShaderManager::GetShader("shadowMap");
		shader.Bind();

		RenderCommand::Enable(RendererEnum::FACE_CULL);
		glCullFace(GL_BACK);

		auto meshView = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
		auto quakeView = scene.Reg().view<TransformComponent, BSPBrushComponent, VisibilityComponent>();
		auto view = scene.Reg().view<TransformComponent, LightComponent, VisibilityComponent>();
		
		for (auto l : view)
		{
			auto [transform, light, visibility] = view.get<TransformComponent, LightComponent, VisibilityComponent>(l);

			if (!light.CastShadows || !visibility.Visible)
				continue;

			if (light.Type == LightType::Directional)
			{
				light.CalculateViewProjection(m_View, m_Projection);

				//for (int i = 0; i < CSM_AMOUNT; i++)
				//{
				//	
				//}
			}
			else if (light.Type == LightType::Point)
			{
				RenderCommand::Disable(RendererEnum::FACE_CULL);

				light.m_Framebuffers[0]->Bind();
				light.m_Framebuffers[0]->Clear();
				{
					Matrix4 lightTransform = Matrix4(1.0f);

					Vec3 pos = transform.GetGlobalPosition();
					
					for (int i = 0; i < CSM_AMOUNT; i++)
					{
						shader.SetMat4("projectionView", light.GetProjection() * glm::inverse(transform.GetGlobalMatrix()));
					}
					//lightSpaceMatrix = light.GetProjection() * glm::inverse(transform.GetGlobalMatrix());
					
					for (auto e : meshView)
					{
						auto [transform, mesh, visibility] = meshView.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
						if (mesh._Model != nullptr && visibility.Visible) {

							for (auto& m : mesh._Model->GetMeshes())
								Renderer::SubmitMesh(m, transform.GetGlobalMatrix());
						}
					}

					Renderer::Flush(&shader, true);
				}
				light.m_Framebuffers[0]->Unbind();
			}
		}
	}

	void SceneRenderer::SkyboxPass() {

		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		Shader shader = ShaderManager::GetShader("skybox");

		gBuffer->Bind();
		gBuffer->DrawBuffer("BaseColor");
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Transform2 skyboxTransform;
		skyboxTransform.position = Engine::GetCurrentScene()->GetCurrentCamera()->GetPosition();
		skyboxTransform.scale = glm::vec3(100.0f * 0.99f);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		Ref<Environment> env = Engine::GetCurrentScene()->GetEnvironment();

		bool hasSky = false;
		auto skyView = Engine::GetCurrentScene()->Reg().view<SkyComponent>();
		for (auto l : skyView)
		{
			SkyComponent& sky = skyView.get<SkyComponent>(l);

			if (!cubemapIsLoaded) {
				g_cubemapTextures["NightSky"].Load("data/textures/NightSky");
				g_cubemapTextures["NightSky"].Bake();
				cubemapIsLoaded = true;
			}
			hasSky = true;
		}

		if (!hasSky && env->CurrentSkyType == SkyType::ProceduralSky) {
			RenderCommand::Clear();
			RenderCommand::SetClearColor(Color{ 0, 0, 0, 1 });
			env->proceduralSkybox->Draw(m_Projection, m_View);
		}
		else if (!hasSky && env->CurrentSkyType == SkyType::ClearColor) {
			RenderCommand::Clear();
			RenderCommand::SetClearColor({ env->ColorClear.x, env->ColorClear.y, env->ColorClear.z, 1 });
		}

		shader.Bind();
		shader.SetMat4("projection", Engine::GetCurrentScene()->GetCurrentCamera()->GetProjectionMatrix());
		shader.SetMat4("view", Engine::GetCurrentScene()->GetCurrentCamera()->GetViewMatrix());
		if (hasSky) {
			shader.SetMat4("modelMatrix", skyboxTransform.to_mat4());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapTextures["NightSky"].GetID());
			Renderer::DrawCube();
		}
		//shader->Stop();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glDepthMask(GL_TRUE);
	}

	void SceneRenderer::OutlinePass(Scene& scene) {

		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		FrameBuffer* outlineFBO = GetFrameBuffer("Outline");

	}

	void SceneRenderer::DebugPass(Scene& scene) {

		//m_ShadingBuffer->Bind();
		g_frameBuffers["GBuffer"].Bind();
		g_frameBuffers["GBuffer"].DrawBuffer("Lighting");
		{
			mLineMesh->Bind();

			Shader shader = ShaderManager::GetShader("debugLine");
			shader.Bind();

			shader.SetMat4("projection", m_Projection);
			shader.SetMat4("view", m_View);

			//bool depthTestState = true;
			for (auto& l : m_DebugLines) {

				shader.SetVec4("color", l.LineColor);
				shader.SetVec3("startPos", l.Start);
				shader.SetVec3("endPos", l.End);

				glLineWidth(l.Width);
				RenderCommand::DrawLines(0, 2);
			}

			//shader->Stop();

			shader = ShaderManager::GetShader("line");
			shader.Bind();
			shader.SetMat4("projection", m_Projection);
			shader.SetFloat("_Opacity", 0.5f);
			
			for (auto& shape : m_DebugShapes) {
				if (shape.DepthTest) {
					glEnable(GL_DEPTH_TEST);
				}
				else {
					RenderCommand::Disable(RendererEnum::DEPTH_TEST);
				}

				shader.SetVec4("Color", shape.LineColor);

				glLineWidth(shape.Width);
				Matrix4 view = m_View;
				//Physics::RigidbodyShapes shapeType = shape.Shape->GetType();
				//switch (shapeType) {
				//case Physics::RigidbodyShapes::BOX:
				//{
				//	const Quat& globalRotation = glm::normalize(shape.Rotation);
				//	const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
				//
				//	view = glm::translate(view, shape.Position) * rotationMatrix;
				//	view = glm::scale(view, reinterpret_cast<Physics::Box*>(shape.Shape.get())->GetSize());
				//
				//	shader->SetMat4("view", view);
				//
				//	m_BoxGizmo->Bind();
				//	RenderCommand::DrawLines(0, 26);
				//	break;
				//}
				//case Physics::RigidbodyShapes::SPHERE:
				//{
				//	const Quat& globalRotation = glm::normalize(shape.Rotation);
				//	const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
				//
				//	view = glm::translate(view, shape.Position) * rotationMatrix;
				//	view = glm::scale(view, Vec3(reinterpret_cast<Physics::Sphere*>(shape.Shape.get())->GetRadius()));
				//	shader->SetMat4("view", view);
				//
				//	//m_SphereGizmo->Bind();
				//	RenderCommand::DrawLines(0, 128);
				//	break;
				//}
				//}
			}

			if (scene.selectedType == _CAMERA)
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

					m_BoxGizmo->Bind();
					RenderCommand::DrawLines(0, 26);
				}
			}
			//if (scene.selectedType == _LIGHT) {
			//	auto lightView = scene.Reg().view<TransformComponent, LightComponent>();
			//	for (auto e : lightView)
			//	{
			//		auto [transform, light] = scene.Reg().get<TransformComponent, LightComponent>(e);
			//		shader->SetFloat("_Opacity", 0.4f);
			//		
			//		
			//		shader->SetVec4("Color", Vec4(1, 1, 1, 1));
			//		shader->SetMat4("view", glm::translate(m_View, Vec3(transform.GetGlobalMatrix()[3])));
			//		if (previousRadiusChange != light.Radius)
			//			sphereMesh.AddSurface(Util::GenerateSphereVertices(light.Radius, 10), Util::GenerateSphereIndices(10));
			//		
			//		sphereMesh.Draw(shader, false);
			//	}
			//}
			//shader->Stop();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
