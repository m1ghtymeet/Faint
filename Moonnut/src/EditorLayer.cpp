#include "EditorLayer.h"
#include <Core/String.h>
#include <Core/GlobalLocator.h>
#include <Debug/Instrumentor.h>

#include <Renderer/Features/EntityRenderFeature.h>
#include <Renderer/Features/LightingRenderFeature.h>
#include <Renderer/Features/DebugLineRenderFeature.h>
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_ShadowMapRenderPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_SkyboxPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_LightingPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_UIPass.h>
#include <Renderer/OpenGL/RenderPasses/GL_PostProcessingPass.h>
#include "Renderer/GL_DebugPass.h"
#include <Renderer/OpenGL/RenderPasses/GL_OutlinePass.h>
#include <AssetManagment/Loader/TextureLoader.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <AssetManagment/MeshManager.h>
#include <AssetManagment/Helper/Serializer.h>
#include "AssetManagment/Config.h"
#include <Scene/Components/CBoxCollider.h>

#include <Scene/Components/CMaterialRenderer.h>

#include <Input/KeyCodes.h>
#include "Input/Input.h"

#include <UI/UIBackEnd.h>
#include <UI/ImUI.h>
#include <FontAwesome5.h>
#include "Util/PlatformUtil.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Math/Math.h"
#include <Engine.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_internal.h>
#include <TextEditor.h>

#include "Panels/FileSystemUI.h"
#include "Panels/MaterialEditor.h"
#include "Panels/AssetView.h"
#include "Panels/Console.h"
#include "Panels/ScriptEditor.h"
#include "Panels/FrameInfo.h"
#include "Panels/ShaderGraph.h"

using namespace Moon;

namespace {
    glm::vec2 g_previousSceneViewport = glm::vec2(0.0f);
    glm::vec2 g_previousGameViewport = glm::vec2(0.0f);
    
    void BlitFrameBuffer(FrameBuffer* srcFrameBuffer, FrameBuffer* dstFrameBuffer, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter) {
        GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
        GLint dstAttachmentSlot = dstFrameBuffer->GetColorAttachmentSlotByName(dstName);
        if (srcAttachmentSlot != GL_INVALID_VALUE && dstAttachmentSlot != GL_INVALID_VALUE) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFrameBuffer->GetHandle());
            glReadBuffer(srcAttachmentSlot);
            glDrawBuffer(dstAttachmentSlot);
            glBlitFramebuffer(0, 0, srcFrameBuffer->GetWidth(), srcFrameBuffer->GetHeight(), 0, 0, dstFrameBuffer->GetWidth(), dstFrameBuffer->GetHeight(), mask, filter);
        }
    }
    inline void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& pos, glm::quat& rot, glm::vec3& scale) {
        pos = glm::vec3(matrix[3]);
        scale.x = glm::length(glm::vec3(matrix[0]));
        scale.y = glm::length(glm::vec3(matrix[1]));
        scale.z = glm::length(glm::vec3(matrix[2]));
        if (scale.x == 0.0f) scale.x = 1.0f;
        if (scale.y == 0.0f) scale.y = 1.0f;
        if (scale.z == 0.0f) scale.z = 1.0f;

        glm::mat3 rotationMatrix(
            glm::vec3(matrix[0]) / scale.x,
            glm::vec3(matrix[1]) / scale.y,
            glm::vec3(matrix[2]) / scale.z
        );
        rot = glm::quat_cast(rotationMatrix);
        rot = glm::normalize(rot); // خیلی مهمه!
    }
}

EditorLayer::EditorLayer(Context& p_context) :
    m_context(p_context),
    m_editorActions(p_context),
    m_panelsManager(m_canvas),
    m_sceneHierarchyPanel("", 1, m_editorActions, m_panelsManager)
{
    FT_PROFILE_FUNCTION();

    // SetupUI
    m_panelsManager.CreatePanel<Menubar>("Menubar");
    Menubar& menubar = m_panelsManager.GetPanelAs<Menubar>("Menubar");
    {
        menubar.AddMenu("File", {
            // Separator - Scene
            { "New Scene", [&]() { m_editorActions.LoadEmptyScene(); }, "Ctrl+N" },
            { "Open Scene...", [&]() { OpenScene(); }, "Ctrl+O"},
            { "Save Scene", [&]() { m_editorActions.SaveSceneChanges(); }, "Ctrl+S"},
            { "Save As...", [&]() { m_editorActions.SaveAsScene(); }, "Ctrl+Shift+S" },
            { "Exit", [&]() { exit(0); }, "Alt+F4" }
            });
        menubar.AddMenu("Edit", {
            { "Undo", []() {}, "Ctrl+Z" },
            { "Redo", []() {}, "Ctrl+Shift+Z" }
            });

        m_panelsManager.CreatePanel<Console>("Console", true, PanelWindowSettings{});
        m_panelsManager.CreatePanel<HardwareInfo>("Hardware Info", false, PanelWindowSettings{});
        m_panelsManager.CreatePanel<Moon::Editor::FrameInfo>("Frame Info", false, PanelWindowSettings{});
        m_panelsManager.CreatePanel<FileBrowser>("File Browser");
        m_panelsManager.CreatePanel<Moon::Editor::MaterialEditor>("Material Editor", false);
        //m_panelsManager.CreatePanel<Moon::Editor::ScriptEditor>("Script Editor", false, PanelWindowSettings{});
        //m_panelsManager.CreatePanel<Moon::Editor::ShaderGraph>("Shader Graph", false, PanelWindowSettings{});
        m_canvas.MakeDockspace(true);
    }

    {
        menubar.AddMenuItem("Build", { "Build at location", [&]() {
            std::string buildPath = Moon::FileDialogs::SaveFile("Build path");
            if (!buildPath.empty())
                m_editorActions.BuildAtLocation("development", buildPath);
            }
            });
        menubar.AddMenu("Tools", {
            { "Hotload Shaders", [&]() { m_sceneViewRenderer->HotReloadShaders(); }},
            { "Template", []() {
                
            }}
        });
    }

    m_context.sceneManager.LoadDefaultScene();

    // Views
    m_sceneViewRenderer = std::make_shared<Moon::Rendering::SceneRenderer>();
    m_sceneViewRenderer->AddFeature<Moon::Rendering::DebugLineRenderFeature>();
    m_sceneViewRenderer->AddPass<Moon::Rendering::ShadowMapRenderPass>("ShadowMap", 0);
    //m_sceneViewRenderer->AddPass<Moon::Rendering::GeometryRenderPass>("Geometry", 1);
    m_sceneViewRenderer->AddPass<Moon::Rendering::LightingRenderPass>("Lighting", 2);
    m_sceneViewRenderer->AddPass<Moon::Rendering::SkyboxRenderPass>("Skybox", 3);
    m_sceneViewRenderer->AddPass<Moon::Rendering::DebugRenderPass>("Debug", 5);

    m_gameViewRenderer = std::make_shared<Moon::Rendering::SceneRenderer>();
    m_gameViewRenderer->AddPass<Moon::Rendering::ShadowMapRenderPass>("ShadowMap", 0);
    //m_gameViewRenderer->AddPass<Moon::Rendering::GeometryRenderPass>("Geometry", 1);
    m_gameViewRenderer->AddPass<Moon::Rendering::LightingRenderPass>("Lighting", 2);
    m_gameViewRenderer->AddPass<Moon::Rendering::SkyboxRenderPass>("Skybox", 3);
    m_gameViewRenderer->AddPass<Moon::Rendering::UIRenderPass>("UI", 4);
    m_gameViewRenderer->AddPass<Moon::Rendering::PostProcessingPass>("PostProcessing", 5);

    // Fonts
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig iconsConfigBold;
    Moon::UI::NormalBold = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 30);
    Moon::UI::Subtitle = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 40);
    Moon::UI::IconSolid = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/fa-solid-900.ttf", 11, 0, icon_ranges);

    UIBackEnd::Init();
    Input::Init(*p_context.window.get());
    p_context.window->SetIcon("data/editor/icons/Moon-logo.png");

    m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;

    m_iconPlay = FTSERVICE(Moon::AssetManagment::TextureManager).CreateResource("data/editor/icons/PlayButton.png");
    m_iconStop = FTSERVICE(Moon::AssetManagment::TextureManager).CreateResource("data/editor/icons/StopButton.png");

    m_editorCamera = EditorCamera(45.0f, 1600 / 900, 0.1f, 500.0f);

    // Load current scene
    if (Moon::Engine::GetProject()->defaultScene->FullPath != "")
        OpenScene(Moon::Engine::GetProject()->defaultScene->FullPath);

    GlobalLocator::Provide<PanelsManager>(m_panelsManager);
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::Update(float p_deltaTime) {
    FT_PROFILE_FUNCTION();

    Input::Update();
    UpdateInputs();
    UpdateCurrentEditorMode(p_deltaTime);

    if (m_previousScene != m_context.sceneManager.GetCurrentScene()) {
        m_previousScene = m_context.sceneManager.GetCurrentScene();
        m_sceneHierarchyPanel.SetScene(m_context.sceneManager.GetCurrentScene());
    }

    if (g_previousSceneViewport != _viewportSize) {
        g_previousSceneViewport = _viewportSize;
        m_editorCamera->UpdateProjection(_viewportSize.x, _viewportSize.y);
    }
    if (g_previousGameViewport != m_gameViewportSize) {
        g_previousGameViewport = m_gameViewportSize;
        if (Moon::CameraComponent* camera = m_context.sceneManager.GetCurrentScene()->FindMainCamera(); camera) {
            camera->MarkAsDirty();
            camera->Update(m_gameViewportSize.x, m_gameViewportSize.y);
        }
    }
    // Update scene
    auto [mx, my] = ImGui::GetMousePos();
    mx -= m_viewportBounds[0].x;
    my -= m_viewportBounds[0].y;
    glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
    my = viewportSize.y - my;
    int mouseX = (int)mx;
    int mouseY = (int)my;

    if (mouseX > 0 && mouseX < _viewportSize.x && mouseY > 0 && mouseY < _viewportSize.y && m_sceneViewportHovered) {
        m_editorCamera->HandleInputs(p_deltaTime);
        
        m_sceneViewRenderer->GetFrameBuffer("GBuffer")->Bind();
        glReadBuffer(m_sceneViewRenderer->GetFrameBuffer("GBuffer")->GetColorAttachmentSlotByName("MousePick"));
        int result;
        glReadPixels(mouseX, mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &result);
        m_hoveredEntity = m_context.sceneManager.GetCurrentScene()->GetEntityByID(result);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    m_editorCamera->OnUpdate(p_deltaTime);

    {
        //auto& assetView = m_panelsManager.GetPanelAs<AssetView>("Asset View");
        //if (assetView.IsOpened())
        //    assetView.Update(p_deltaTime);

        if (m_panelsManager.GetPanelAs<Moon::Editor::FrameInfo>("Frame Info").IsOpened())
            m_panelsManager.GetPanelAs<Moon::Editor::FrameInfo>("Frame Info").Update(nullptr, p_deltaTime);
    }

    UIBackEnd::Update();
    OnImGuiRender();
    m_panelsManager.GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor").Update(p_deltaTime);
    UIBackEnd::EndFrame();
}

void EditorLayer::UpdateCurrentEditorMode(float p_deltaTime) {
    if (auto editorMode = m_editorActions.GetEditorMode();
            editorMode == EditorActions::EEditorMode::PLAY)
        UpdatePlayMode(p_deltaTime);
    else
        UpdateEditMode(p_deltaTime);

    {
        m_context.sceneManager.Update();
    }
}

void EditorLayer::UpdatePlayMode(float p_deltaTime) {

    auto currentScene = m_context.sceneManager.GetCurrentScene();

    m_context.physicsEngine->Update(p_deltaTime);

    m_context.audioEngine->Update();

    currentScene->FixedUpdate(p_deltaTime);

    currentScene->Update(p_deltaTime);
    for (const auto camera : m_context.sceneManager.GetCurrentScene()->GetFastAccessComponents().cameras) {
        TransformComponent* transform = camera->owner.transform;
        camera->camera.SetPosition(transform->GetGlobalPosition());
        camera->camera.SetRotationQ(transform->GetGlobalRotation());
        camera->camera.Update();
    }   

    if (Input::KeyPressed(Key::Escape)) {
        ImGui::SetWindowFocus("Scene View");
        m_editorActions.StopPlaying();
    }
}

void EditorLayer::UpdateEditMode(float p_deltaTime) {

    for (const auto camera : m_context.sceneManager.GetCurrentScene()->GetFastAccessComponents().cameras) {
        TransformComponent* transform = camera->owner.transform;
        camera->camera.SetPosition(transform->GetGlobalPosition());
        camera->camera.SetRotationQ(transform->GetGlobalRotation());
        camera->camera.Update();
    }

    if (Input::KeyPressed(Key::F5)) {
        //ImGui::SetWindowFocus("Game View");
        m_editorActions.StartPlaying();
        m_sceneHierarchyPanel.SetSelectedEntity({});
        if (m_panelsManager.GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor").IsOpened()) {
            m_panelsManager.GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor").Close();
        }
    }
}

void EditorLayer::UpdateInputs()
{
    // Input Keys
    bool control = Input::KeyDown(Key::LeftControl) || Input::KeyDown(Key::RightControl);
    bool shift = Input::KeyDown(Key::LeftShift) || Input::KeyDown(Key::RightShift);
    
    if (control) {
        if (Input::KeyPressed(Key::N)) {
            m_editorActions.LoadEmptyScene();
            m_sceneHierarchyPanel.SetScene(m_context.sceneManager.GetCurrentScene());
        }
        if (Input::KeyPressed(Key::O)) OpenScene();
        if (shift) { if (Input::KeyPressed(Key::S)) m_editorActions.SaveAsScene(); }
        if (Input::KeyPressed(Key::S)) m_editorActions.SaveSceneChanges();
        //if (Input::KeyPressed(Key::D)) OnDuplicateEntity();
        if (Input::KeyPressed(Key::T)) {
            m_sceneViewRenderer->HotReloadShaders();
        }
    }
    if (Input::KeyPressed(Key::Delete)) {
        if (GImGui->ActiveId == 0) {
            Entity* selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
            if (selectedEntity) {
                m_sceneHierarchyPanel.SetSelectedEntity(nullptr);
                m_context.sceneManager.GetCurrentScene()->DestroyEntity(*selectedEntity);
            }
        }
    }

    if (m_sceneViewportFocused && m_sceneViewportHovered && !m_editorCamera->m_using) {
        if (Input::KeyPressed(Key::W) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
        if (Input::KeyPressed(Key::E) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::ROTATE;
        if (Input::KeyPressed(Key::R) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::SCALE;
    }

    if (Input::LeftMousePressed()) {
        if (m_sceneViewportHovered && !ImGuizmo::IsOver() && !Input::KeyDown(Key::LeftAlt)) {
            if (m_hoveredEntity) {
                m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity);
            }
            else
                m_sceneHierarchyPanel.SetSelectedEntity(nullptr);
        }
    }

    if (ImGui::IsMouseDoubleClicked(0)) {
        if (m_sceneViewportHovered && m_hoveredEntity && !ImGuizmo::IsOver()) {
            m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity->GetParent());
            //{
            //    const glm::vec3 targetPos = m_hoveredEntity->transform->GetGlobalPosition();
            //    const float distance = 5.0f; // فاصله‌ی فوکوس (قابل تنظیم)
            //
            //    glm::vec3 dir = glm::normalize(m_editorCamera->GetForward());
            //    glm::vec3 newPos = targetPos - dir * distance;
            //    float yaw = glm::degrees(atan2(dir.x, dir.z));
            //    float pitch = glm::degrees(asin(-dir.y));
            //
            //    m_editorCamera->SetPosition(newPos);
            //    m_editorCamera->SetRotation(glm::vec3(pitch, yaw, 0.0f));
            //}
        }
    }

    m_sceneViewRenderer->GetPass<Moon::Rendering::DebugRenderPass>("Debug").selected = m_sceneHierarchyPanel.GetSelectedEntity();
}

int selected = 0;
void EditorLayer::OnImGuiRender() {
    FT_PROFILE_FUNCTION();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSize = style.WindowMinSize.x;
    m_canvas.Draw();
    style.WindowMinSize.x = minWinSize;

    {
        m_panelsManager.GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor").SetScene(*m_context.sceneManager.GetCurrentScene());
    }

    m_sceneHierarchyPanel.Draw();

    UI_Views();

    /* ================================================================== */
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    Engine::EndDraw();
}

void EditorLayer::UI_Views() {
    auto* currentScene = m_context.sceneManager.GetCurrentScene();
    if (!currentScene) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    if (ImGui::Begin("Scene View"))
    {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        if (_viewportSize != *((glm::vec2*)&viewportPanelSize)) {
            _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        }
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_viewportBounds[0].x;
        my -= m_viewportBounds[0].y;
        glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;
        //m_editorActions.ProcessHandleEvents(m_editorCamera.value(), m_sceneViewRenderer->GetDescriptor<ColliderGizmoState>(), mouseX, mouseY, m_sceneViewRenderer->GetFrameDescriptor().renderWidth, m_sceneViewRenderer->GetFrameDescriptor().renderHeight);

        auto* fm = m_sceneViewRenderer->GetFrameBuffer("GBuffer").get();
        if (fm->GetWidth() != _viewportSize.x || fm->GetHeight() != _viewportSize.y)
            fm->Resize(_viewportSize.x, _viewportSize.y);

        Rendering::Data::FrameDescriptor frameDescriptor;
        frameDescriptor.renderWidth = _viewportSize.x;
        frameDescriptor.renderHeight = _viewportSize.y;
        frameDescriptor.camera = m_editorCamera;

        m_sceneViewRenderer->AddDescriptor<Rendering::SceneRenderer::SceneDescriptor>({ m_context.sceneManager.GetCurrentScene() });

        m_sceneViewRenderer->BeginFrame(frameDescriptor);
        m_sceneViewRenderer->DrawFrame();
        m_sceneViewRenderer->EndFrame();

        ImGuizmo::BeginFrame();
        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        m_sceneViewportFocused = ImGui::IsWindowFocused();
        m_sceneViewportHovered = ImGui::IsWindowHovered();

        uint32_t textureID = m_sceneViewRenderer->GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("FinalLighting");
        ImGui::Image((void*)textureID, ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SCENE")) {
                char* path = (char*)payload->Data;
                OpenScene(FileSystem::AbsoluteToRelative(path));
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MODEL")) {
                std::string droppedPath = static_cast<const char*>(payload->Data);
                droppedPath = String::ReplaceSlash(droppedPath);

                std::string rootName = std::filesystem::path(droppedPath).stem().string();
                Entity& rootEntity = m_context.sceneManager.GetCurrentScene()->CreateEntity(rootName);
                rootEntity.transform->SetLocalPosition({ 0, 0, 0 });

                Model* model = Loaders::ModelLoader::Create(droppedPath, false);
                if (!model || model->GetMeshIndices().empty()) {
                    delete model;
                    ImGui::EndDragDropTarget();
                }

                Rendering::Material* defaultMaterial = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
				auto& matComp = rootEntity.AddComponent<MaterialRendererComp>();
                for (auto meshIndex : model->GetMeshIndices()) {
                    matComp.AddMaterialByIndex(AssetManagment::MeshManager::GetMeshByIndex(meshIndex)->materialIndex, *defaultMaterial);
                }
                auto& meshComp = rootEntity.AddComponent<MeshRendererComponent>();
                meshComp.SetModel(model);
                meshComp.ModelPath = droppedPath;

                //delete model;
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_PREFAB")) {
                std::string path = static_cast<const char*>(payload->Data);
                Entity& newEntity = m_context.sceneManager.GetCurrentScene()->InstantiatePrefab(path);
            }
            ImGui::EndDragDropTarget();
        }

        {
            const float distance = 10.0f;
            bool showOverlay = true;
            int corner = 1;
            ImGuiIO& io = ImGui::GetIO();
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

            windowFlags |= ImGuiWindowFlags_NoMove;
            ImGuiViewport* viewport = ImGui::GetWindowViewport();
            ImVec2 workAreaPos = ImGui::GetCurrentWindow()->Pos;
            ImVec2 workAreaSize = ImGui::GetCurrentWindow()->Size;
            ImVec2 windowPos = ImVec2((corner & 1) ? (workAreaPos.x + workAreaSize.x - distance) : (workAreaPos.x + distance), (corner & 2) ? (workAreaPos.y + workAreaSize.y - distance) : (workAreaPos.y + distance));
            ImVec2 windowPosPivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos({ windowPos.x, windowPos.y + 25.0f }, ImGuiCond_Always, windowPosPivot);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::SetNextWindowBgAlpha(0.35f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
            if (ImGui::Begin("RendererOrderBar", &showOverlay, windowFlags)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(20, 20, 20, 60));

                const char* items[] = { "Lighting", "Albedo", "Normal", "RMA", "WorldPositionSpace" };
                ImGui::SetNextItemWidth(150);
                if (ImGui::BeginCombo("##Output", items[selected])) {
                    for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                        bool isSelected = (selected == i);
                        if (ImGui::Selectable(items[i], isSelected))
                            selected = i;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

        // Gizmos
        Entity* selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_gizmoType != -1) {

            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            // Entity transform
            auto* tc = selectedEntity->GetComponent<TransformComponent>();
            glm::mat4 transform = tc->GetTransform().GetWorldMatrix();

            // Snapping
            const float translateionSnap = 0.5f;
            const float rotationSnap = 15.0f;
            const float scaleSnap = 0.1f;

            bool snap = Input::KeyDown(Key::LeftControl);
            float snapValues[3] = { translateionSnap, translateionSnap, translateionSnap };
            switch (m_gizmoType) {
            case ImGuizmo::OPERATION::TRANSLATE:
                snapValues[0] = snapValues[1] = snapValues[2] = translateionSnap;
                break;
            case ImGuizmo::OPERATION::ROTATE:
                snapValues[0] = snapValues[1] = snapValues[2] = rotationSnap;
                break;
            case ImGuizmo::OPERATION::SCALE:
                snapValues[0] = snapValues[1] = snapValues[2] = scaleSnap;
                break;
            }

            ImGuizmo::Manipulate(
                glm::value_ptr(m_editorCamera->GetViewMatrix()),
                glm::value_ptr(m_editorCamera->GetProjectionMatrix()),
                (ImGuizmo::OPERATION)m_gizmoType,
                (ImGuizmo::MODE)m_gizmoMode,
                glm::value_ptr(transform),
                NULL,
                snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing()) {
                glm::mat4 finalTransform = transform;
                glm::vec3 newLocalPos;
                glm::quat newLocalRot;
                glm::vec3 newLocalScale;
                if (tc->GetTransform().HasParent()) {
                    const auto& parent = tc->owner.GetParent()->transform;
                    glm::mat4 parentMatrix = parent->GetGlobalMatrix();
                    glm::mat4 localMatrix = glm::inverse(parentMatrix) * finalTransform;
                    DecomposeMatrix(localMatrix, newLocalPos, newLocalRot, newLocalScale);
                }
                else {
                    DecomposeMatrix(finalTransform, newLocalPos, newLocalRot, newLocalScale);
                }
                tc->SetLocalPosition(newLocalPos);
                tc->SetLocalRotation(newLocalRot);
                tc->SetLocalScale(newLocalScale);
            }
        }
    }
    ImGui::End();

    //windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    //ImGui::SetNextWindowClass(&windowClass);
    if (ImGui::Begin("Game View")) {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_gameViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        if (m_gameViewportSize != *((glm::vec2*)&viewportPanelSize)) {
            m_gameViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        }

        auto* fm = m_gameViewRenderer->GetFrameBuffer("GBuffer").get();
        if (fm->GetWidth() != m_gameViewportSize.x || fm->GetHeight() != m_gameViewportSize.y)
            fm->Resize(m_gameViewportSize.x, m_gameViewportSize.y);

        Rendering::Data::FrameDescriptor desc;
        desc.renderWidth = m_gameViewportSize.x;
        desc.renderHeight = m_gameViewportSize.y;

        if (auto camera = currentScene->FindMainCamera())
            desc.camera = camera->camera;

        m_gameViewRenderer->AddDescriptor<Rendering::SceneRenderer::SceneDescriptor>({ m_context.sceneManager.GetCurrentScene() });

        m_gameViewRenderer->BeginFrame(desc);
        m_gameViewRenderer->DrawFrame();
        m_gameViewRenderer->EndFrame();

        //uint32_t textureID = gameRenderer->GetFrameDescriptor().outputBuffer->GetColorAttachmentHandleByName("Color");
        uint32_t textureID = m_gameViewRenderer->GetPass<Moon::Rendering::PostProcessingPass>("PostProcessing").GetID();
		//uint32_t textureID = m_gameViewRenderer->GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("FinalLighting");
        ImGui::Image((void*)textureID, ImVec2{ m_gameViewportSize.x, m_gameViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    }
    ImGui::End();

    UI_Toolbar();
    ImGui::PopStyleVar();

    {
        //auto& assetView = m_panelsManager.GetPanelAs<AssetView>("Asset View");
        //if (assetView.IsOpened())
        //    assetView.Render();
    }
    {
        /*ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Engine Documentation - Complate Guide")) {
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Moon Engine dev%s - Error Code Refrence", FAINT_VERSION);
            ImGui::Separator();
            ImGui::TextWrapped("This list contains all possible errors in the engine. Each error includes a code, a description, a common cause, and a solution.");
            ImGui::Spacing();

            static char search[128] = "";
            ImGui::Text("Search:"); ImGui::SameLine();
            ImGui::InputText("##search", search, IM_ARRAYSIZE(search));

            if (ImGui::BeginTabBar("ErrorCategories")) {
                if (ImGui::BeginTabItem("Mesh & Geometry")) {
                    ImGui::BeginChild("MeshErrors", ImVec2(0, 0), true);

                    const auto* filter = search; if (filter[0] == '\0') filter = nullptr;
                    auto ErrorEntry = [&](const char* code, const char* title, const char* desc, const char* cause, const char* fix, ImVec4 color = ImVec4(1, 0.3f, 0.3f, 1)) {
                        if (filter && strstr(title, filter) == nullptr && strstr(desc, filter) == nullptr) return;
                        if (ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                            ImGui::Text("Code: %s", code);
                            ImGui::PopStyleColor();
                            ImGui::TextWrapped("Description: %s", desc);
                            ImGui::TextWrapped("Common Cause: %s", cause);
                            ImGui::TextWrapped("Solution: %s", fix);
                            ImGui::Separator();
                        }
                    };

                    ErrorEntry("MESH-001", "Mesh has no indices",
                        "This mesh was loaded without indices and is rendered using glDrawArrays (less efficient).",
                        "The model was exported from a non-indexed format (like OBJ without faces), or the indices were removed during loading.",
                        "In Blender/FBX: make sure Triangulate is enabled and the model has indices. Or generate indices in code.");

                    ErrorEntry("MESH-002", "Mesh has no normal map",
                        "u_HasNormal = 0 → normal map is not used.",
                        "The material has no normal map, the path is incorrect, or the texture failed to load.",
                        "In the material, assign a Normal Map, check the file path, or use a default texture (128,128,255).");

                    ErrorEntry("MESH-003", "Tangent missing or invalid",
                        "TBN matrix is constructed incorrectly → the normal map appears inverted or black.",
                        "The model lacked Tangent/Bitangent data during export, or Assimp failed to generate them.",
                        "In Assimp: enable aiProcess_CalcTangentSpace. In Blender: Object Data > UV Maps > Generate Tangents.");

                    ErrorEntry("MESH-004", "BaseVertex overflow / negative",
                        "glDrawElementsBaseVertex throws GL_INVALID_VALUE and the model disappears.",
                        "baseVertex was converted to int32_t and exceeded 2 billion → wrapped around to a negative number.",
                        "Use uint32_t for baseVertex and never convert it to int32_t.");

                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }*/
    }
}

void EditorLayer::UI_Toolbar() {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    if (ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove)) {

        float panelPaddingX = 8.0f;
        float panelPaddingY = 4.0f;
        ImGui::SetCursorPos(ImVec2(panelPaddingX, panelPaddingY));

        float iconSize = 20.0f;
        float padding = 6.0f;
        float buttonSize = iconSize + padding * 2.0f;
        float spacing = 8.0f;

        Assets::Texture* icon =
            m_editorActions.GetEditorMode() == EditorActions::EEditorMode::EDIT
            ?
            m_iconPlay :
            m_iconStop;

        if (ImGui::ImageButton((ImTextureID)icon->GetTexture().GetID(), ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1), 0)) {
            if (m_editorActions.GetEditorMode() == EditorActions::EEditorMode::EDIT)
                m_editorActions.StartPlaying();
            else if (m_editorActions.GetEditorMode() == EditorActions::EEditorMode::PLAY)
                m_editorActions.StopPlaying();
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL)) m_gizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD)) m_gizmoMode = ImGuizmo::WORLD;
    }
    ImGui::PopStyleVar(2);
    ImGui::End();
}

void EditorLayer::NewProject() {

    if (Engine::GetProject() && Engine::GetProject()->Exists())
        Engine::GetProject()->Save();

    std::string selectedProject = FileDialogs::SaveFile("Project file\0*.project");

    if (selectedProject.empty())
        return;

    if (!String::EndsWith(selectedProject, ".project"))
        selectedProject += ".project";

    auto backslashSplits = String::Split(selectedProject, '\\');
    auto fileName = backslashSplits[backslashSplits.size() - 1];

    std::string finalPath = String::Split(selectedProject, '.')[0];

    // We need to create a folder
    if (const auto& dirPath = finalPath; !std::filesystem::create_directory(dirPath))
    {
        // Sound we continue?
        HZ_CORE_ERROR("Failed creating project directory: " + dirPath);
    }

    finalPath += "/" + fileName;

    std::shared_ptr<Project> project = std::make_shared<Project>(String::Split(fileName, '.')[0], "Your Description", finalPath);
    Engine::LoadProject(project);
}

void EditorLayer::OpenProject()
{
    std::string projectPath = FileDialogs::OpenFile("Project file\0*.project");

    if (projectPath.empty())
        return;

    FileSystem::SetRootDirectory(FileSystem::GetParentPath(projectPath));
    Ref<Project> project = Project::Load(projectPath);
    project->FullPath = projectPath;
    Engine::LoadProject(project);
}

void EditorLayer::OpenScene() {

    std::string filepath = FileDialogs::OpenFile("Moon Scene (*.scene)\0*.scene\0");
    if (!filepath.empty()) {
        OpenScene(filepath);
    }
}

void EditorLayer::OpenScene(const std::filesystem::path& path) {
    if (path.extension().string() != ".scene") {
        HZ_WARN("Could not load {0} - not a scene file", path.filename().string());
        return;
    }
    std::string finalPath = String::Split(path.string(), '.')[0];
    m_sceneHierarchyPanel.SetSelectedEntity(nullptr);
    m_context.sceneManager.LoadScene(finalPath, true);
    m_sceneHierarchyPanel.SetScene(m_context.sceneManager.GetCurrentScene());
    m_context.window->SetTitle("Moon Editor - dev" + std::string(FAINT_VERSION) + " " + std::filesystem::path(Moon::Engine::GetProject()->defaultScene->FullPath).stem().string());
}