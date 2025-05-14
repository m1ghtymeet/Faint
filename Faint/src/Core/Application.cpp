#include "hzpch.h"
#include "Application.h"

#include "Debug/Log.h"
#include "Time.h"

#include "Input/Input.h"

#include "FileSystem/FileSystem.h"

#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Engine.h"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace Faint {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& aSpecification)
			: m_Specification(aSpecification) {

		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		if (!aSpecification.Headless)
			m_window = Window::Get();
	}

	void Application::Run() {

		HZ_PROFILE_FUNCTION();

		while (m_window->ShouldClose()) {

			HZ_PROFILE_SCOPE("RunLoop");

			float time = static_cast<float>(glfwGetTime());
			Time timestep = time - m_lastFrameTime;
			m_lastFrameTime = time;
			Engine::SetTime(timestep.GetDeltaTime());

			if (!m_minimized)
			{
				HZ_PROFILE_SCOPE("LayerStack OnUpdate");
				for (Layer* layer : m_layerStack)
					layer->OnUpdate(timestep);
			}
		}
	}

	void Application::Close() {

		for (auto& layer : m_layerStack)
			layer->OnDetach();
	}

	/*void Application::OnEvent(Event& e) {

		HZ_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_layerStack.end(); it != m_layerStack.begin(); ) {
			if (e.Handled)
				break;
			(*--it)->OnEvent(e);
		}
	}*/

	void Application::PushLayer(Layer* layer) {

		HZ_PROFILE_FUNCTION();

		m_layerStack.PushLayer(layer);
		layer->OnAttach();
		
	}

	void Application::PushOverlay(Layer* layer) {

		HZ_PROFILE_FUNCTION();

		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	//bool Application::OnWindowClose(WindowCloseEvent& e) {
	//	m_running = false;
	//	return true;
	//}
	//bool Application::OnWindowResize(WindowResizeEvent& e) {
	//
	//	HZ_PROFILE_FUNCTION();
	//
	//	if (e.GetWidth() == 0 || e.GetHeight() == 0) {
	//		m_minimized = true;
	//		return false;
	//	}
	//	m_minimized = false;
	//	Renderer::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
	//
	//	return false;
	//}
}