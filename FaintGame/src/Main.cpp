#include <Hazel.h>
#include <Core/EntryPoint.h>

#include <Engine.h>
#include "FileSystem/FileSystem.h"
#include "Scene/SceneSerializer.h"
#include "Renderer/Renderer2D.h"

#include "Core/Application.h"
#include "Math/Math.h"

#include <imgui.h>

struct LaunchSettings {
	int32_t monitor = -1;
	Faint::Vec2 resolution = { 1920, 1080 };
	std::string windowTitle = "Hazel Runtime";
	std::string projectPath;
};

namespace Faint {

	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer() : Layer("RuntimeLayer") {}
		virtual ~RuntimeLayer() = default;

		virtual void OnAttach() override
		{
			Engine::Init();

			std::string projectPath = "Data/User/Game.ini";
			FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));

			auto project = Project::Load(projectPath);

			m_scene = CreateRef<Scene>();
			
			SceneSerializer serializer(m_scene);
			if (serializer.Deserialize("Data/User/Assets/scenes/level01.scene")) {
				Engine::SetCurrentScene(m_scene);
			}

			Engine::LoadProject(project);
			Engine::EnterPlayMode();
		}

		virtual void OnDetach() override {}

		void OnUpdate(Time time) override
		{
			Engine::Update(time);
			Engine::Draw();

			const auto& windowSize = Engine::GetCurrentWindow()->GetSize();
			//Renderer2D::BeginDraw(windowSize);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			_viewportSize = Vec2(viewport->Size.x, viewport->Size.y);
			m_scene->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

			Ref<Framebuffer2> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();

			if (framebuffer->GetSize() != _viewportSize)
			{
				framebuffer->QueueResize(_viewportSize);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
			ImGui::Begin("Game", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
			{
				ImGui::Image((void*)Window::Get()->GetFrameBuffer()->GetTexture()->GetID(), ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
			}
			ImGui::End();
			ImGui::PopStyleVar(2);

			Engine::EndDraw();
		}

		//void OnEvent(Event& e) override {}

	private:
		Ref<Scene> m_scene;

		Vec2 _viewportSize = Vec2(1280, 720);
	};

	class Runtime : public Application {
	private:
		LaunchSettings m_settings;
	public:
		Runtime() : Application(ApplicationSpecification()) {
			PushLayer(new RuntimeLayer());
		}
	};

	Application* CreateApplication() {
		return new Runtime();
	}
}