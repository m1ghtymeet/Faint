#include "Application.h"
#include <Core/Time.h>
#include <Analytics/GPUInfo.h>
#include <Core/GlobalLocator.h>

Application::Application(const std::filesystem::path& m_projectFolder) :
	m_context(m_projectFolder),
	m_editor(m_context) {
}

Application::~Application() {

}

void Application::Run() {
	
	Moon::Tools::Time time;

	while (IsRunning()) {

		m_editor.Update(time.GetDeltaTime());
		time.Update();
		m_context.window->SwapBuffers();
	}
}

bool Application::IsRunning() const {
	return m_context.window->ShouldClose();
}