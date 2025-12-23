#pragma once

#include "PanelWindow.h"
#include <Scene/Scene.h>
#include <Renderer/SceneRenderer.h>

class AView : public PanelWindow {
public:
	AView(
		const std::string& p_title,
		bool p_opened,
		const PanelWindowSettings& p_windowSettings
	);

	virtual void Update(float p_deltaTime);

protected:
	void _Base_Draw() override;

public:
	virtual void InitFrame();

	void Render();

	virtual void DrawFrame();

	virtual Moon::Camera* GetCamera() = 0;

	virtual Moon::Scene* GetScene() = 0;

	const Moon::Rendering::SceneRenderer& GetRenderer() const;

protected:
	Moon::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor();

protected:
	//Moon::Rendering::Framebuffer m_framebuffer;
	std::unique_ptr<Moon::Rendering::SceneRenderer> m_renderer;
	bool m_windowHovered = false;
};