#pragma once

#include <Core/Time.h>
#include "Core/Context.h"
#include "Core/EditorActions.h"
#include "Core/PanelsManager.h"
#include <Scene/Entity.h>
#include <Types/Renderer/Texture.h>
#include <Renderer/SceneRenderer.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>
#include "EditorCamera.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/HardwareInfo.h"

#include <memory>

class EditorLayer {
public:
    EditorLayer(Context& p_context);
    ~EditorLayer();

    void Update(float p_deltaTime);
    void OnImGuiRender();

private:
    // Update
    void UpdateInputs();
    void UpdateCurrentEditorMode(float p_deltaTime);
    void UpdatePlayMode(float p_deltaTime);
    void UpdateEditMode(float p_deltaTime);

    // UI
    void UI_Views();
    void UI_Toolbar();

    // Scene Managment
    void OpenScene();
    void OpenScene(const std::filesystem::path& path);

    void NewProject();
    void OpenProject();

public:
    PanelsManager m_panelsManager;

private:
    Context& m_context;
    EditorActions m_editorActions;
    Canvas m_canvas;

    std::optional<Moon::EditorCamera> m_editorCamera;
    Moon::Entity* m_hoveredEntity = nullptr;
    Moon::Scene* m_previousScene = nullptr;
    std::shared_ptr<Moon::Rendering::SceneRenderer> m_sceneViewRenderer;
    std::shared_ptr<Moon::Rendering::SceneRenderer> m_gameViewRenderer;
    std::shared_ptr<Moon::FrameBuffer> m_sceneFinalOutput;
    std::shared_ptr<Moon::FrameBuffer> m_gameFinalOutput;
private:
    bool m_isSelectedEntity = false;
    bool m_sceneViewportFocused = false, m_sceneViewportHovered = false;
    glm::vec2 _viewportSize = glm::vec2(0);
    glm::vec2 m_gameViewportSize = glm::vec2(0);
    glm::vec2 m_viewportBounds[2];

    int m_gizmoType = -1;
    int m_gizmoMode = 0;

    // Panels
    SceneHierarchyPanel m_sceneHierarchyPanel;

    // Editor resource
    Moon::Assets::Texture* m_iconPlay = nullptr;
    Moon::Assets::Texture* m_iconStop = nullptr;
};