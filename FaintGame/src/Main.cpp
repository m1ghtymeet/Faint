#include <Faint.h>
#include <Core/EntryPoint.h>

#include <Engine.h>
#include "FileSystem/FileSystem.h"
#include "Renderer/SceneRenderer.h"

#include "Core/Application.h"
#include "Math/Math.h"
#include <imgui.h>

namespace Faint {
	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer() : Layer("RuntimeLayer") {}
		virtual ~RuntimeLayer() = default;

		virtual void OnAttach() override
		{
			Engine::Init();
			
			std::string projectPath = "C:/Users/Atmosfer-PC/Documents/Engine Projects/Example/Example.project";
			FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

			auto project = Project::Load(projectPath);
			
			m_scene = CreateRef<Scene>();
			
			//SceneSerializer serializer(m_scene);
			//if (serializer.Deserialize("C:\\Users\\Atmosfer-PC\\Documents\\Engine Projects\\Example\\Assets\\scenes\\test.scene")) {
			//	Engine::SetCurrentScene(m_scene);
			//}
			
			Engine::LoadProject(project);
			Engine::EnterPlayMode();
		}

		virtual void OnDetach() override {}

		void OnUpdate(Time time) override {
			Engine::Update(time);
			Engine::Draw();

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			_viewportSize = Vec2(viewport->Size.x, viewport->Size.y);
			//_viewportSize = Vec2(Engine::GetCurrentWindow()->GetWidth(), Engine::GetCurrentWindow()->GetHeight());
			m_scene->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

			//ImGui::Begin("##aaa");
			//uint32_t textureID = SceneRenderer::GetFrameBuffer("FinalImage")->GetColorAttachmentHandleByName("Color");
			//ImGui::Image((void*)textureID, viewport->Size, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//ImGui::End();

			Engine::EndDraw();
		}
	private:
		Ref<Scene> m_scene;

		Vec2 _viewportSize = Vec2(1280, 720);
	};

	class Runtime : public Application {
	public:
		Runtime() : Application(ApplicationSpecification()) {
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication() {
		return new Runtime();
	}
}