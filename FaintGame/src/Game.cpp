#include "Game.h"
#include <Core/String.h>
#include <AssetManagment/Project.h>
#include <Input/Input.h>
#include <FileSystem/FileSystem.h>
#include <Renderer/Features/EntityRenderFeature.h>
#include <Renderer/Features/LightingRenderFeature.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_ShadowMapRenderPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_LightingPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_UIPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_PostProcessingPass.h>
#include <AssetManagment/MeshManager.h>

Game::Game(Context& p_context) :
	m_context(p_context) {

	Moon::AssetManagment::MeshManager::Init();

	m_sceneRenderer = new Moon::Rendering::SceneRenderer();
	m_sceneRenderer->AddFeature<Moon::Rendering::EntityRenderFeature>();
	m_sceneRenderer->AddFeature<Moon::Rendering::LightingRenderFeature>();
	m_sceneRenderer->AddPass<Moon::Rendering::ShadowMapRenderPass>("ShadowMap", 0);
	m_sceneRenderer->AddPass<Moon::Rendering::GeometryRenderPass>("Geometry", 1);
	m_sceneRenderer->AddPass<Moon::Rendering::LightingRenderPass>("Lighting", 2);
	//m_sceneRenderer->AddPass<Moon::Rendering::UIRenderPass>("UI", 3);
	//m_sceneRenderer->AddPass<Moon::Rendering::PostProcessingPass>("PostProcessing", 4);

	//m_sceneRenderer->GetFrameBuffer("GBuffer")->Resize(1600 / 2, 900 / 2);

	Moon::Input::Init(*p_context.window.get());

	auto project = Moon::Project::Load((std::filesystem::current_path() / "data" / "game" / "Example.project").string());
	Moon::FileSystem::SetRootDirectory(Moon::FileSystem::GetParentPath(project->FullPath));

	//if (!std::filesystem::exists(Moon::FileSystem::Root)) {
	//	HZ_CORE_WARN("[FileSystem] Game data directory not found: {}", Moon::FileSystem::Root);
	//	return;
	//}
	//
	//{
	//	HZ_CORE_INFO("[FileSystem] Scanning for .mault files in: {}", Moon::FileSystem::Root);
	//
	//	Moon::FileSystem::UnmountPak();
	//
	//	for (const auto& entry : std::filesystem::directory_iterator(Moon::FileSystem::Root)) {
	//		if (!entry.is_regular_file()) continue;
	//
	//		const std::string& filename = entry.path().filename().string();
	//		if (entry.path().extension() == ".mault") {
	//			HZ_CORE_TRACE("[FileSystem] Found file: {}", filename);
	//			Moon::FileSystem::MountPak(entry.path().string());
	//		}
	//	}
	//	HZ_CORE_INFO("[FileSystem] Finished mounting .mault files.");
	//}

	std::string scenePath = Moon::String::ReplaceSlash(project->defaultScene->FullPath);
	m_context.sceneManager.LoadScene(Moon::String::Split(scenePath, '.')[0], true);
	m_context.sceneManager.GetCurrentScene()->Play();
}

Game::~Game() {
	m_context.sceneManager.UnloadCurrentScene();
}

void Game::Update(float p_deltaTime) {

	Moon::Input::Update();
	m_context.sceneManager.Update();
	m_context.physicsEngine->Update(p_deltaTime);
	m_context.audioEngine->Update();

	if (auto currentScene = m_context.sceneManager.GetCurrentScene()) {
		if (auto camera = currentScene->FindMainCamera()) {

			// ========== Update ==========
			if (m_context.window->HasFocus()) {
				Moon::TransformComponent* transform = camera->owner.GetComponent<Moon::TransformComponent>();
				camera->camera.SetPosition(transform->GetGlobalPosition());
				camera->camera.SetRotationQ(transform->GetGlobalRotation());
				camera->camera.Update();

				{
					currentScene->FixedUpdate(p_deltaTime);
				}
				{
					currentScene->Update(p_deltaTime);
				}
			}

			// ========= Rendering =========
			m_sceneRenderer->AddDescriptor<Moon::Rendering::SceneRenderer::SceneDescriptor>({ currentScene });

			Moon::Rendering::Data::FrameDescriptor frameDescriptor;
			frameDescriptor.renderWidth = 1600;
			frameDescriptor.renderHeight = 900;
			frameDescriptor.camera = camera->camera;

			m_sceneRenderer->BeginFrame(frameDescriptor);
			m_sceneRenderer->DrawFrame();
			m_sceneRenderer->EndFrame();
		}
		//m_sceneRenderer->GetPass<Moon::Rendering::PostProcessingPass>("PostProcessing").BlitBackBuffer(1600, 900);
		m_sceneRenderer->GetFrameBuffer("GBuffer")->BlitToBackBuffer("FinalLighting", 1600, 900);
	}

	m_context.window->SwapBuffers();
}
