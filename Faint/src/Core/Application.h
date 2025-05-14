#pragma once

#include "Base.h"
#include "Window.h"
#include "Core/LayerStack.h"

int main(int argc, char** argv);

namespace Faint {

	struct ApplicationSpecification
	{
		std::string Name = "Application";
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
		bool VSync = false;
		std::string WorkingDirectory = "";
		bool Headless = false;
	};

	class Application {
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application() {};

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void Run();
		void Close();

		//ImGuiLayer* GetImGuiLayer() { return m_imGuiLayer; }
		static Application& Get() { return *s_Instance; }

		inline Ref<Window> GetWindow() const { return m_window; }
	private:
		//bool OnWindowClose(WindowCloseEvent& e);
		//bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		LayerStack m_layerStack;
		bool m_running = true;

		Ref<Window> m_window;
		Ref<Scene> m_currentScene;

		//ImGuiLayer* m_imGuiLayer;
		
		bool m_minimized = false;

		float m_lastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in Client
	Application* CreateApplication();
}