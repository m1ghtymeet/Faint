#include "Application.h"

#include <Core/Time.h>

Application::Application() : m_game(m_context) {

}

Application::~Application() {
}

void Application::Run() {

	Moon::Tools::Time time;

	while (IsRunning()) {

		m_game.Update(time.GetDeltaTime());
		time.Update();
	}
}

bool Application::IsRunning() const {
	return m_context.window->ShouldClose();
}