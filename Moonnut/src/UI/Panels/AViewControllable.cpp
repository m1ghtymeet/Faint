#include "AViewControllable.h"
#include <imgui.h>

AViewControllable::AViewControllable(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	AView(p_title, p_opened, p_windowSettings) {
	ResetCameraTransform();
}

void AViewControllable::Update(float p_deltaTime) {
	if (m_windowHovered) {
		m_cameraController.UpdateProjection(m_size.x, m_size.y);
		m_cameraController.HandleInputs(p_deltaTime);
	}
	AView::Update(p_deltaTime);
}

void AViewControllable::InitFrame() {
	AView::InitFrame();
}

void AViewControllable::ResetCameraTransform() {
	m_camera.SetPosition({ -10.0f, 3.0f, 10.0f });
	m_camera.SetRotation({ 0.0f, 135.0f, 0.0f });
}

Moon::Camera* AViewControllable::GetCamera() {
	return &m_camera;
}
