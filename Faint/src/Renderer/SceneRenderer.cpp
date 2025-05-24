#include "SceneRenderer.h"
#include "RenderCommand.h"
#include "ShadersManager.h"
#include "Renderer.h"
#include "Engine.h"
#include "Scene/Components.h"
#include "Scene/Components/ParentComponent.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Types/ShadowMap.h"
#include "Input/Input.h"
#include "Util/Util.h"

namespace Faint::SceneRenderer {
	std::unordered_map<std::string, FrameBuffer> g_frameBuffers;
	std::unordered_map<std::string, CubemapTexture> g_cubemapTextures;

	glm::vec3 g_lightDir = glm::normalize(glm::vec3(-2.0f, 4.0f, -1.0f));
	glm::vec3 g_lightPos = -g_lightDir * 10.0f;

	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
	glm::mat4 lightView = glm::lookAt(
		g_lightPos,
		glm::vec3(0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	glm::mat4 g_lightSpaceMatrix;

	ShadowMap* m_ShadowMap;
	Matrix4 m_Projection, m_View;
	Vec3 m_CamPos;

	Ref<Mesh> mLineMesh;
	Ref<BoxGizmo> m_BoxGizmo;

	void Init() {

		float resolutionScale = Engine::GetProject()->Settings.ResolutionScale;
		Vec2 defaultResolution = Vec2(Engine::GetCurrentWindow()->viewportWidth, Engine::GetCurrentWindow()->viewportHeight);
		Vec2 finalImageResolution = Vec2(Engine::GetCurrentWindow()->viewportWidth * resolutionScale, Engine::GetCurrentWindow()->viewportHeight * resolutionScale);
		g_frameBuffers = std::unordered_map<std::string, FrameBuffer>();
		g_frameBuffers["GBuffer"] = FrameBuffer("GBuffer", defaultResolution.x, defaultResolution.y);
		g_frameBuffers["GBuffer"].CreateAttachment("BaseColor", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Normal", GL_RGBA16F);
		g_frameBuffers["GBuffer"].CreateAttachment("RMA", GL_RGBA8);
		g_frameBuffers["GBuffer"].CreateAttachment("Lighting", GL_RGB16F);
		g_frameBuffers["GBuffer"].CreateAttachment("WorldSpacePosition", GL_RGBA32F);
		g_frameBuffers["GBuffer"].CreateAttachment("MousePick", GL_R32I);
		g_frameBuffers["GBuffer"].CreateDepthAttachment(GL_DEPTH_COMPONENT32F);

		g_frameBuffers["Outline"] = FrameBuffer("Outline", defaultResolution.x, defaultResolution.y);
		//g_frameBuffers["Outline"].CreateAttachment("Mask", GL_R8);
		//g_frameBuffers["Outline"].CreateAttachment("Result", GL_R8);
		g_frameBuffers["Outline"].CreateAttachment("Color", GL_RGB16F);

		g_frameBuffers["FinalImage"] = FrameBuffer("FinalImage", finalImageResolution.x, finalImageResolution.y);
		g_frameBuffers["FinalImage"].CreateAttachment("Color", GL_RGB16F);

		m_ShadowMap = new ShadowMap(2048, 2048);

		// Generate debug meshes
		std::vector<Vertex> lineVertices{
			{ Vec3(0, 0, 0), Vec3(0, 0, 0), Vec2(0, 0) },
			{ Vec3(1, 1, 1), Vec3(0, 0, 0), Vec2(0, 0) }
		};
		std::vector<uint32_t> lineIndices{ 0, 1 };

		// Debug shapes
		mLineMesh = CreateRef<Mesh>();
		mLineMesh->AddSurface(lineVertices, lineIndices);

		m_BoxGizmo = CreateRef<BoxGizmo>();
		m_BoxGizmo->CreateMesh();
	}

	void Cleanup() {
	}

	void Update(const Time time) {
		// Delete debug shapes that are dead
		//if (m_DebugLines.size() > 0)
		//{
		//	std::erase_if(m_DebugLines, [](const DebugLine& line)
		//		{
		//			return line.Life < 0.0f;
		//		});
		//
		//	for (auto& line : m_DebugLines) {
		//		line.Life -= time;
		//	}
		//}
		//if (m_DebugShapes.size() > 0)
		//{
		//	std::erase_if(m_DebugShapes, [](const DebugShape& shape)
		//		{
		//			return shape.Life < 0.0f;
		//		});
		//
		//	for (auto& shape : m_DebugShapes)
		//	{
		//		shape.Life -= time;
		//	}
		//}
		if (Engine::IsPlayMode()) {
			Renderer2D::Update();
		}
	}

	void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos) {
		m_Projection = projection;
		m_View = view;
		m_CamPos = camPos;
	}

	void RenderScene(Scene& scene, bool renderUI) {

		const Vec2 framebufferResolution = Vec2(Window::Get()->viewportWidth, Window::Get()->viewportHeight);

		glDisable(GL_DITHER);
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
		}

		g_lightSpaceMatrix = lightProjection * lightView;
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ShadowMap->GetID());
			Shader shadowMapShader = ShaderManager::GetShader("shadowMap");
			shadowMapShader.Bind();
			shadowMapShader.SetMat4("projectionView", g_lightSpaceMatrix);
			
			auto view = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
			for (auto e : view) {
				auto [transform, mesh, visiblity] = view.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
				shadowMapShader.SetMat4("modelMatrix", transform.GetGlobalMatrix());
				if (mesh._Model && visiblity.Visible) {
					for (Ref<Mesh> m : mesh._Model->GetMeshes()) {
						m->Draw(&shadowMapShader, false);
					}
				}
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		//SkyboxPass();

		// Save previous Matrix
		//auto modelView = scene.Reg().view<TransformComponent, MeshRendererComponent, VisibilityComponent>();
		//for (auto e : modelView) {
		//	auto [transform, mesh, visibility] = modelView.get<TransformComponent, MeshRendererComponent, VisibilityComponent>(e);
		//	Entity entity = { (entt::entity)e, Engine::GetCurrentScene().get() };
		//	if (!entity.IsValid())
		//		continue;
		//
		//	if (mesh._Model && visibility.Visible) {
		//		transform.PreviousMatrix = m_Projection * m_View * transform.GetGlobalMatrix();
		//	}
		//}

		if (renderUI)
			g_frameBuffers["GBuffer"].Resize(framebufferResolution.x, framebufferResolution.y);
		GBufferPass(scene);
		ShadingPass(scene);
		
		//if (!renderUI) {
			BlitFrameBuffer(&g_frameBuffers["GBuffer"], &g_frameBuffers["FinalImage"], "Lighting", "Color", GL_COLOR_BUFFER_BIT, GL_LINEAR);
			BlitToDefaultFrameBuffer(&g_frameBuffers["FinalImage"], "Color", GL_COLOR_BUFFER_BIT, GL_NEAREST);
		//}
		//if (renderUI) {
		//	DebugPass(scene);
		//}

		//g_frameBuffers["Outline"].Resize(framebufferResolution.x, framebufferResolution.y);
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

		//RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	}

	FrameBuffer* GetFrameBuffer(const std::string& name) {
		auto it = g_frameBuffers.find(name);
		return (it != g_frameBuffers.end()) ? &it->second : nullptr;
	}

	void DrawDebugLine(const Vec3& start, const Vec3& end, const Color& color, float life, float width) {

		//DebugLine debugLine = {
		//	start,
		//	end,
		//	color,
		//	life,
		//	width,
		//	true
		//};
		//
		//m_DebugLines.push_back(debugLine);
	}

	void DrawDebugShape(const Vec3& position, const Quat& rotation, PxShape* shape, const Color& color, float life, float width)
	{
		//DebugShape debugShape{
		//	.Position = position,
		//	.Rotation = rotation,
		//	.LineColor = color,
		//	.Life = life,
		//	.Width = width,
		//	.DepthTest = true,
		//	//.Shape = 
		//};
		//
		//m_DebugShapes.push_back(debugShape);
	}

	void GBufferPass(Scene& scene) {
		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		gBuffer->Bind();
		gBuffer->DrawBuffers({ "BaseColor", "Normal", "RMA", "WorldSpacePosition", "MousePick" });
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

					//gBufferShader.SetMat4("model", transform.GetGlobalMatrix());
					//gBufferShader.SetInt("aEntityID", (int)e);
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
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool cubemapIsLoaded = false;
	void ShadingPass(Scene& scene) {

		// ZoneScoped
		g_frameBuffers["GBuffer"].Bind();
		g_frameBuffers["GBuffer"].DrawBuffer("Lighting");
		glViewport(0, 0, g_frameBuffers["GBuffer"].GetWidth(), g_frameBuffers["GBuffer"].GetHeight());
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			Ref<Environment> env = scene.GetEnvironment();

			Shader shader = ShaderManager::GetShader("deferred");
			shader.Bind();
			shader.SetMat4("projection", m_Projection);
			shader.SetMat4("view", m_View);
			shader.SetVec3("CamPosition", m_CamPos);
			shader.SetFloat("time", Engine::GetTime());
			shader.SetMat4("lightProjectionView", g_lightSpaceMatrix);

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
				[](const LightDistance& a, const LightDistance& b) {
					return a.distance < b.distance;
				});

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
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());

			Renderer::DrawQuad();
		}
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowPass(Scene& scene)
	{
		
	}

	void SkyboxPass() {

		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		Shader shader = ShaderManager::GetShader("skybox");

		gBuffer->Bind();
		gBuffer->DrawBuffer("BaseColor");
		glViewport(0, 0, gBuffer->GetWidth(), gBuffer->GetHeight());
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		//if (!hasSky && env->CurrentSkyType == SkyType::ProceduralSky) {
		//	RenderCommand::Clear();
		//	RenderCommand::SetClearColor(Color{ 0, 0, 0, 1 });
		//	env->proceduralSkybox->Draw(m_Projection, m_View);
		//}
		//else if (!hasSky && env->CurrentSkyType == SkyType::ClearColor) {
		//	RenderCommand::Clear();
		//	RenderCommand::SetClearColor({ env->ColorClear.x, env->ColorClear.y, env->ColorClear.z, 1 });
		//}

		shader.Bind();
		shader.SetMat4("projection", Engine::GetCurrentScene()->GetCurrentCamera()->GetProjectionMatrix());
		shader.SetMat4("view", Engine::GetCurrentScene()->GetCurrentCamera()->GetViewMatrix());
		if (hasSky) {
			shader.SetMat4("modelMatrix", skyboxTransform.to_mat4());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapTextures["NightSky"].GetID());
			Renderer::DrawCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OutlinePass(Scene& scene) {

		FrameBuffer* gBuffer = GetFrameBuffer("GBuffer");
		FrameBuffer* outlineFBO = GetFrameBuffer("Outline");

	}

	void DebugPass(Scene& scene) {

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
			//for (auto& l : m_DebugLines) {
			//
			//	shader.SetVec4("color", l.LineColor);
			//	shader.SetVec3("startPos", l.Start);
			//	shader.SetVec3("endPos", l.End);
			//
			//	glLineWidth(l.Width);
			//	RenderCommand::DrawLines(0, 2);
			//}

			//shader->Stop();

			shader = ShaderManager::GetShader("line");
			shader.Bind();
			shader.SetMat4("projection", m_Projection);
			shader.SetFloat("_Opacity", 0.5f);

			//for (auto& shape : m_DebugShapes) {
			//	if (shape.DepthTest) {
			//		glEnable(GL_DEPTH_TEST);
			//	}
			//	else {
			//		RenderCommand::Disable(RendererEnum::DEPTH_TEST);
			//	}
			//
			//	shader.SetVec4("Color", shape.LineColor);
			//
			//	glLineWidth(shape.Width);
			//	Matrix4 view = m_View;
			//	//Physics::RigidbodyShapes shapeType = shape.Shape->GetType();
			//	//switch (shapeType) {
			//	//case Physics::RigidbodyShapes::BOX:
			//	//{
			//	//	const Quat& globalRotation = glm::normalize(shape.Rotation);
			//	//	const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
			//	//
			//	//	view = glm::translate(view, shape.Position) * rotationMatrix;
			//	//	view = glm::scale(view, reinterpret_cast<Physics::Box*>(shape.Shape.get())->GetSize());
			//	//
			//	//	shader->SetMat4("view", view);
			//	//
			//	//	m_BoxGizmo->Bind();
			//	//	RenderCommand::DrawLines(0, 26);
			//	//	break;
			//	//}
			//	//case Physics::RigidbodyShapes::SPHERE:
			//	//{
			//	//	const Quat& globalRotation = glm::normalize(shape.Rotation);
			//	//	const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
			//	//
			//	//	view = glm::translate(view, shape.Position) * rotationMatrix;
			//	//	view = glm::scale(view, Vec3(reinterpret_cast<Physics::Sphere*>(shape.Shape.get())->GetRadius()));
			//	//	shader->SetMat4("view", view);
			//	//
			//	//	//m_SphereGizmo->Bind();
			//	//	RenderCommand::DrawLines(0, 128);
			//	//	break;
			//	//}
			//	//}
			//}

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
	}
}