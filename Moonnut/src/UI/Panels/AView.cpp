#include "AView.h"

#include <Types/Renderer/Texture.h>
#include <Renderer/Data/FrameDescriptor.h>
#include <imgui.h>

AView::AView(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_title, p_opened, p_windowSettings)
	//m_framebuffer(p_title)
{
	//Moon::Rendering::FramebufferUtil::SetupFramebuffer(
	//	m_framebuffer,
	//	1280,
	//	720,
	//	true, true, true
	//);

	//const auto tex = m_framebuffer.GetAttachment<Moon::Rendering::Texture>(Moon::Rendering::EFramebufferAttachment::COLOR);

}

void AView::Update(float p_deltaTime)
{
}

void AView::_Base_Draw() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	PanelWindow::_Base_Draw();
	ImGui::PopStyleVar();
}

void AView::InitFrame() {
	m_renderer->AddDescriptor<Moon::Rendering::SceneRenderer::SceneDescriptor>(
		CreateSceneDescriptor()
	);
}

void AView::Render() {

	// Safe Size
	constexpr float titleBarHeight = 25.0f;
	const auto& size = GetSize();
	auto [winWidth, winHeight] = std::pair<uint16_t, uint16_t>{
		static_cast<uint16_t>(size.x),
		static_cast<uint16_t>(std::max(0.0f, size.y - titleBarHeight))
	};

	auto camera = GetCamera();
	auto scene = GetScene();
		
	if (winWidth > 0 && winHeight > 0 && camera && scene) {

		InitFrame();

		Moon::Rendering::Data::FrameDescriptor frameDescriptor;
		frameDescriptor.renderWidth = winWidth;
		frameDescriptor.renderHeight = winHeight;
		frameDescriptor.camera = *camera;
		//frameDescriptor.outputBuffer = m_framebuffer;

		m_renderer->BeginFrame(frameDescriptor);
		DrawFrame();
		m_renderer->EndFrame();
	}
}

void AView::DrawFrame() {
	m_renderer->DrawFrame();
}

const Moon::Rendering::SceneRenderer& AView::GetRenderer() const {
	return *m_renderer.get();
}

Moon::Rendering::SceneRenderer::SceneDescriptor AView::CreateSceneDescriptor() {
	auto scene = GetScene();
	FT_CORE_ASSERT(scene, "No scene assigned to this view!");
	return {
		scene
	};
}
