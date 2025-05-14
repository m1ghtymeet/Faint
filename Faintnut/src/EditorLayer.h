#pragma once
#include <Hazel.h>
#include "imgui.h"

#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ContentBrowserPanel.h"
#include "Panel/FileSystemUI.h"
#include "Windows/NewProjectWindow.h"

#include "Scripting/ScriptingEngineNet.h"

namespace Faint {

    class EditorLayer : public Layer {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void OnUpdate(Time time) override;
        virtual void OnImGuiRender() override;
    private:
        void UpdateInputs();

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveAsScene();

        void NewProject();
        void OpenProject();

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();
        void OnDuplicateEntity();

        // UI Panels
        void UI_Toolbar();
    private:
        //Ref<Framebuffer2> m_framebuffer;
        Ref<Scene> SceneSnapshot;
        Ref<Scene> m_editorScene, m_runtimeScene;
        Entity m_hoveredEntity, m_selectedEntity;
        NewProjectWindow* _NewProjectWindow;

        //EditorCamera m_editorCamera;
        std::vector<CompilationError> errors;

        bool _viewportFocused = false, _viewportHovered = false;
        glm::vec2 _viewportSize = glm::vec2(0);
        glm::vec2 m_viewportBounds[2];

        int m_gizmoType = -1;

        enum class SceneState {
            Edit = 0, Play = 1, Simulate = 2,
        };
        SceneState m_sceneState = SceneState::Edit;

        // Panels
        SceneHierarchyPanel m_sceneHierarchyPanel;
        //ContentBrowserPanel m_contentBrowserPanel;
        FileSystemUI m_fileBrowserPanel;

        // Editor resource
        Ref<Texture> m_iconPlay, m_iconStop, m_iconSimulate;
    };
}