#pragma once

#include "AView.h"
#include "../../EditorCamera.h"

class AViewControllable : public AView {
public:
	AViewControllable(
		const std::string& p_title,
		bool p_opened,
		const PanelWindowSettings& p_windowSettings
	);

	virtual void Update(float p_deltaTime) override;

	virtual void InitFrame() override;

	virtual void ResetCameraTransform();

	virtual Moon::Camera* GetCamera();

protected:
	Moon::Camera m_camera;
	Moon::EditorCamera m_cameraController;
};