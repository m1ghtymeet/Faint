#include "AssetView.h"
#include <Renderer/Features/EntityRenderFeature.h>
#include <Renderer/Features/LightingRenderFeature.h>
#include <Renderer/Features/DebugLineRenderFeature.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_ShadowMapRenderPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_LightingPass.h>
#include "../Renderer/GL_DebugPass.h"

#include <imgui.h>

AssetView::AssetView(const std::string& p_title, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	AViewControllable(p_title, p_opened, p_windowSettings) {

    m_renderer = std::make_unique<Moon::Rendering::SceneRenderer>();
    m_renderer->AddFeature<Moon::Rendering::EntityRenderFeature>();
    m_renderer->AddFeature<Moon::Rendering::DebugLineRenderFeature>();
    m_renderer->AddFeature<Moon::Rendering::LightingRenderFeature>();
    m_renderer->AddPass<Moon::Rendering::ShadowMapRenderPass>("ShadowMap", 0);
    m_renderer->AddPass<Moon::Rendering::GeometryRenderPass>("Geometry", 1);
    m_renderer->AddPass<Moon::Rendering::LightingRenderPass>("Lighting", 2);
    m_renderer->AddPass<Moon::Rendering::DebugRenderPass>("Debug", 5);

    m_assetEntity = &m_scene.CreateEntity("Asset");
    m_modelRenderer = &m_assetEntity->AddComponent<Moon::MeshRendererComponent>();
}

Moon::Scene* AssetView::GetScene() {
	return &m_scene;
}

void AssetView::SetModel(Moon::Model& p_model)
{
}

void AssetView::_Draw_Impl() {
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    if (m_size != *((glm::vec2*)&viewportPanelSize)) {
        m_size = { viewportPanelSize.x, viewportPanelSize.y };
    }
    m_windowHovered = ImGui::IsWindowHovered();

    uint32_t textureID = m_renderer->GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("Lighting");
	ImGui::Image((void*)textureID, { m_size.x, m_size.y }, { 0, 1 }, { 1, 0 });
}
