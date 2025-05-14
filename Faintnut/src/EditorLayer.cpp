#include "EditorLayer.h"

#include "Renderer/RenderCommand.h"
#include "Input/Input.h"

#include "Scene/SceneSerializer.h"
#include "Scene/Components/ParentComponent.h"
#include "Renderer/SceneRenderer.h"
#include "Renderer/ShadersManager.h"

#include "Threading/JobSystem.h"

#include "Util/PlatformUtil.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Math/Math.h"
#include "Engine.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Faint {

    extern const std::filesystem::path s_assetPath;
    std::string currentScenePath = "";
    GLuint sceneFBO, sceneTexture;
    GLuint blurFBO1, blurFBO2, blurTex1, blurTex2;

    EditorLayer::EditorLayer() : Layer("EditorLayer") {

    }

    void EditorLayer::OnAttach() {

        HZ_PROFILE_FUNCTION();
        
        m_iconPlay = CreateRef<Texture>("data/editor/icons/PlayButton.png");
        m_iconStop = CreateRef<Texture>("data/editor/icons/StopButton.png");
        m_iconSimulate = CreateRef<Texture>("data/editor/icons/SimulateButton.png");

        auto project = Project::New();
        try {
            //Project::Load("assets/scenes/Example.project");
            Engine::LoadProject(project);
        }
        catch (std::exception exception) {
            HZ_CORE_ERROR("Error Loading project!");
        }

        Engine::Init();
        Engine::GetCurrentWindow()->SetTitle("Faint Editor");
        Engine::GetCurrentWindow()->SetIcon("data/editor/icons/faint-logo.png");
        

        m_editorScene = CreateRef<Scene>();
        //m_activeScene = m_editorScene;
        Engine::SetCurrentScene(m_editorScene);

        m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
        m_sceneHierarchyPanel.SetContext(m_editorScene);

        // Windows
        _NewProjectWindow = new NewProjectWindow();
    
        {
            auto createTexture = [](GLuint& tex) {
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                };

            // Scene
            glGenFramebuffers(1, &sceneFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
            createTexture(sceneTexture);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);

            // Blur pass 1
            glGenFramebuffers(1, &blurFBO1);
            glBindFramebuffer(GL_FRAMEBUFFER, blurFBO1);
            createTexture(blurTex1);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex1, 0);

            // Blur pass 2
            glGenFramebuffers(1, &blurFBO2);
            glBindFramebuffer(GL_FRAMEBUFFER, blurFBO2);
            createTexture(blurTex2);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex2, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
        }
    }

    void EditorLayer::OnDetach() {

        HZ_PROFILE_FUNCTION();
    }

    void EditorLayer::OnUpdate(Time time) {

        HZ_PROFILE_FUNCTION();

        //auto framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
        Engine::GetCurrentScene()->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

        // Resize
        if (glm::vec2(Window::Get()->viewportWidth, Window::Get()->viewportHeight) != _viewportSize) {
            Window::Get()->viewportWidth = _viewportSize.x;
            Window::Get()->viewportHeight = _viewportSize.y;
            //framebuffer->QueueResize(_viewportSize);
        }

        Engine::Update(time);
        Engine::Draw();

        // Update scene
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_viewportBounds[0].x;
        my -= m_viewportBounds[0].y;
        glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (!Engine::IsPlayMode() /* && ImGui::GetIO().WantCaptureMouse*/ && _viewportHovered && _viewportFocused && !ImGuizmo::IsUsing()
            && mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            bool foundSomethingToSelect = false;
            auto* gBuffer = SceneRenderer::GetFrameBuffer("GBuffer");
            if (!foundSomethingToSelect) {
                gBuffer->Bind();
                glReadBuffer(gBuffer->GetColorAttachmentSlotByName("MousePick"));
                int result;
                glReadPixels((int)mouseX, (int)mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &result);
                {
                    m_hoveredEntity = result == 1045220557 ? Entity() : Entity((entt::entity)(result - 1), Engine::GetCurrentScene().get() );
                    SceneRenderer::m_hoveredEntityID = result;
                    foundSomethingToSelect = true;
                }
            
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        OnImGuiRender();

        Engine::EndDraw();

        Input::Update();
        UpdateInputs();
    }

    void EditorLayer::OnImGuiRender() {

        HZ_PROFILE_FUNCTION();

        //auto framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();

        static bool dockSpaceOpen = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSize = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (!_NewProjectWindow->HasCreatedProject()) {
            _NewProjectWindow->Draw();
            if (Engine::GetProject()->DefaultScene->FullPath != "")
                OpenScene(Engine::GetProject()->DefaultScene->FullPath);
        }

        style.WindowMinSize.x = minWinSize;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project")) {
                    NewProject();
                }
                if (ImGui::MenuItem("Open Project...")) {
                    OpenProject();
                }

                ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4{ .6f, .6f, .6f, 1 });
                ImGui::Separator();
                ImGui::PopStyleColor();

                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    NewScene();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    OpenScene();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    SaveScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                    SaveAsScene();
                }

                if (ImGui::MenuItem("Exit")) Faint::Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Build"))
            {
                if (ImGui::MenuItem("Build")) {
                    std::string buildPath = FileDialogs::SaveFile("Build path");

                    if (buildPath != "")
                    {
                        bool failed = false;

                        HZ_CORE_INFO("Preparing to build at location: \"" + buildPath + "\"");

                        std::filesystem::remove_all(buildPath);

                        if (std::filesystem::create_directory(buildPath))
                        {
                            HZ_CORE_INFO("Build directory Created");

                            if (std::filesystem::create_directory(buildPath + "\\Data\\")) {

                                HZ_CORE_INFO("Data directory Created");
                                HZ_CORE_WARN(buildPath);

                                if (std::filesystem::create_directory(buildPath + "\\Data\\User\\")) {

                                    HZ_CORE_INFO("Data directory Created");

                                    std::error_code err;

                                    std::filesystem::copy(FileSystem::Root + "Example.project", buildPath + "\\Data\\User\\Game.ini", err);

                                    if (!err) {

                                        HZ_CORE_INFO("Data\\User\\Game.ini file generated");

                                        std::filesystem::copy(Engine::GetProject()->AssetDirectory.c_str(), buildPath + "\\Data\\User\\Assets\\", std::filesystem::copy_options::recursive, err);

                                        //if (!std::filesystem::exists(buildPath + "Data\\User\\Assets\\"))) {
                                        //}

                                        if (!err) {

                                            HZ_CORE_INFO("Data\\User\\Assets\\ directory copied");

                                            std::filesystem::copy(Engine::GetProject()->ScriptDirectory.c_str(), buildPath + "\\Data\\User\\Scripts\\", std::filesystem::copy_options::recursive, err);

                                            if (!err) {

                                                HZ_CORE_INFO("Data\\User\\Scripts\\ directory copied");
                                            }
                                            else {
                                                HZ_CORE_INFO("Data\\User\\Scripts\\ directory failed to copy");
                                                failed = true;
                                            }
                                        }
                                        else {
                                            HZ_CORE_ERROR("Data\\User\\Assets\\ directory failed to copy");
                                            failed = true;
                                        }
                                    }
                                    else {
                                        HZ_CORE_ERROR("Data\\User\\Game.ini file failed to generate");
                                        failed = true;
                                    }

                                    std::string builderFolder = "builder\\" + Engine::GetProject()->Settings.Configuration + "\\";

                                    if (std::filesystem::exists(builderFolder)) {

                                        std::error_code err;

                                        std::filesystem::copy(builderFolder, buildPath, err);

                                        if (!err) {

                                            HZ_CORE_INFO("Builder data (DLLs and EXEcutatble) copied");

                                            std::filesystem::rename(buildPath + "FNGame.exe", buildPath + Engine::GetProject()->Name.c_str(), err);

                                            if (!err) {

                                                HZ_CORE_INFO("Game EXEcutable renamed to " + Engine::GetProject()->Name);
                                            }
                                            else {

                                                HZ_CORE_ERROR("Game EXEcutable failed to rename!");
                                                failed = true;
                                            }
                                        }
                                        else {

                                            HZ_CORE_ERROR("Builder data (DLLs and EXEcutable) failed to copy");
                                            failed = true;
                                        }
                                    }
                                    else {
                                        const std::string buildConfiguration = Engine::GetProject()->Settings.Configuration == "Development" ? "Debug" : "Release";
                                        HZ_CORE_ERROR("Builder folder for " + Engine::GetProject()->Settings.Configuration + " not found. Verify you have compiled Engine source code in '" + buildConfiguration + "' configuration.");
                                        /*\"" + "Debug" + "\"*/
                                        failed = true;
                                    }
                                }
                            }
                        }
                        else {
                            HZ_CORE_ERROR("Build directory failed to create");
                            failed = true;
                        }

                        if (failed)
                        {
                            std::filesystem::remove_all(buildPath);
                            //FileDialogs::MsgBox("An error occured during the building of your game.\nCheck the console for more information", "Build Failure", MB_ICONERROR | MB_OK);
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Build Shaders", "Ctrl+T"))
                    ShaderManager::RebuildShaders();
                
                if (ImGui::MenuItem("Generate Visual Solution (If you want to use C# Scripting)")) {
                    ScriptingEngineNet::Get().GenerateSolution(FileSystem::Root, Engine::GetProject()->Name);
                    //HZ_CORE_INFO("Generated Solution.");
                }
                
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        ImGui::End();

        m_sceneHierarchyPanel.OnImGuiRender();
        if (_NewProjectWindow->HasCreatedProject())
            m_sceneHierarchyPanel.DrawStatusBar();
        //m_contentBrowserPanel.OnImGuiRender();
        m_fileBrowserPanel.Draw();

        // Logger
        if (ImGui::Begin("Console")) {
            if (ImGui::Button("Clear", ImVec2(60, 28))) {
                
            }
            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_FILTER, ImVec2(30, 28))) {
                ImGui::OpenPopup("filter_popup");
            }
            ImGui::SameLine();

            if (ImGui::BeginPopup("filter_popup")) {
                ImGui::SeparatorText("Filters");
                ImGui::EndPopup();
            }
            ImGui::SameLine();

            ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable;
            if (ImGui::BeginTable("LogTable", 3, flags)) {
                ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));

                

                ImGui::PopStyleVar();
            }
            ImGui::EndTable();
        }
        ImGui::End();

        /*ImGui::Begin("Stats");

        std::string name = "None";
        if (m_hoveredEntity)
            name = m_hoveredEntity.GetComponent<NameComponent>().name;
        ImGui::Text("Hovered Entity: %s", name.c_str());

        ImGui::End();*/

        //ImGui::Begin("Test");
        //uint32_t textureID = Engine::GetCurrentScene()->m_sceneRenderer->GetOutlineBuffer().GetTexture(GL_COLOR_ATTACHMENT0)->GetID();
        //ImGui::Image((void*)textureID, ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        //ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");
        {
            ImGuizmo::BeginFrame();
            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            _viewportFocused = ImGui::IsWindowFocused();
            _viewportHovered = ImGui::IsWindowHovered();
            //Application::Get().GetImGuiLayer()->SetBlockEvents(_viewportFocused || _viewportHovered);

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

            if (_viewportSize != *((glm::vec2*)&viewportPanelSize)) {
                _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
                Engine::GetCurrentScene()->OnViewportResize(_viewportSize.x, _viewportSize.y);
            }
            uint32_t textureID = SceneRenderer::GetFrameBuffer("FinalImage")->GetColorAttachmentHandleByName("Color");
            //uint32_t textureID = SceneRenderer::GetShadingBuffer()->GetTexture()->GetID();
            ImGui::Image((void*)textureID, ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Scene")) {
                    char* path = (char*)payload->Data;
                    OpenScene(path);
                }
                ImGui::EndDragDropTarget();
            }

            Engine::GetCurrentScene()->selectedType = NONE;

            // Gizmos
            Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
            if (selectedEntity && m_gizmoType != -1) {

                if (selectedEntity.HasComponent<CameraComponent>())
                    Engine::GetCurrentScene()->selectedType = _CAMERA;
                if (selectedEntity.HasComponent<LightComponent>())
                    Engine::GetCurrentScene()->selectedType = _LIGHT;

                // Draw Grid
                float windowWidth = (float)ImGui::GetWindowWidth();
                float windowHeight = (float)ImGui::GetWindowHeight();

                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
                //ImGuizmo::DrawGrid(
                //    glm::value_ptr(Engine::.GetViewMatrix()),
                //    glm::value_ptr(m_editorCamera.GetProjection()),
                //    glm::value_ptr(glm::identity<glm::mat4>()), 100.0f);

                // Camera

                // Runtime camera from entity
                // auto cameraEntity = m_activeScene->GetPrimaryCameraEntity();
                // const glm::mat4& cameraProjection = cameraEntity.GetComponent<CameraComponent>().camera.GetProjection();
                // glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<Transform>().to_mat4());

                // Editor camera
                const glm::mat4& cameraProjection = Engine::GetCurrentScene()->m_EditorCamera->GetProjectionMatrix();
                glm::mat4 cameraView = Engine::GetCurrentScene()->m_EditorCamera->GetViewMatrix();

                // Entity transform
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 transform = tc.GetGlobalMatrix();
                glm::vec3 originalRotation = Vec3(tc.GetGlobalRotation().x, tc.GetGlobalRotation().y, tc.GetGlobalRotation().z);

                // Snapping
                bool snap = Input::KeyPressed(Key::LeftControl);
                float snapValue = 0.5f;
                // Snap to 45 degrees for rotation
                if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                float snapValues[3] = { snapValue, snapValue, snapValue };

                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL,
                    glm::value_ptr(transform), NULL,
                    snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing()) {

                    Matrix4 localTransform = Matrix4(transform);

                    ParentComponent& parent = selectedEntity.GetComponent<ParentComponent>();
                    if (parent.HasParent) {
                        const auto& parentTransformComponent = parent.Parent.GetComponent<TransformComponent>();
                        const Matrix4& parentTransform = parentTransformComponent.GetGlobalMatrix();
                        localTransform = glm::inverse(parentTransform) * localTransform;
                    }

                    // Decompose local transform
                    float decomposedPosition[3];
                    float decomposedRotation[3];
                    float decomposedScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), decomposedPosition, decomposedRotation, decomposedScale);

                    const auto& localPosition = Vec3(decomposedPosition[0], decomposedPosition[1], decomposedPosition[2]);
                    const auto& localScale = Vec3(decomposedScale[0], decomposedScale[1], decomposedScale[2]);

                    localTransform[0] /= localScale.x;
                    localTransform[1] /= localScale.y;
                    localTransform[2] /= localScale.z;
                    const auto& rotationMatrix = Matrix3(localTransform);
                    const Quat& localRotation = glm::normalize(Quat(rotationMatrix));

                    const Matrix4& rotationMatrix4 = glm::mat4_cast(localRotation);
                    const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
                    const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localPosition);
                    const Matrix4& newLocalTransform = translationMatrix * rotationMatrix4 * scaleMatrix;

                    tc.SetLocalPosition(localPosition);

                    if (m_gizmoType != ImGuizmo::SCALE)
                        tc.SetLocalRotation(localRotation);

                    tc.SetLocalScale(localScale);
                    tc.SetLocalMatrix(newLocalTransform);
                }
            }

        }
        ImGui::End();
        ImGui::PopStyleVar();

        UI_Toolbar();
    }

    void EditorLayer::UI_Toolbar() {

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float size = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

        bool hasPlayButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play;
        bool hasSimulateButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate;
        bool hasPauseButton = m_sceneState != SceneState::Edit;

        //if (Engine::IsPlayMode()) {
        //
        //    Color color = Engine::GetProject()->Settings.PrimaryColor;
        //    ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, 1.0f });
        //    if (ImGui::Button(ICON_FA_PAUSE, ImVec2(size, size)) || Input::KeyPressed(Key::F6)) {
        //
        //        Engine::SetGameState(GameState::Paused);
        //        m_sceneHierarchyPanel.SetStatusMessage("Paused");
        //    }
        //    ImGui::PopStyleColor();
        //
        //    if (ImGui::BeginItemTooltip())
        //    {
        //        ImGui::Text("Pause game (F6)");
        //        ImGui::EndTooltip();
        //    }
        //}
        //else {
            //bool hasPlayButton;

            //if (Engine::GetGameState() == GameState::Paused) {

                //Color color = Engine::GetProject()->Settings.PrimaryColor;
                //ImGui::PushStyleColor(ImGuiCol_Text, { color.r, color.g, color.b, 1.0f });
                //Ref<Texture> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
                //hasPlayButton = ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5);
                //ImGui::PopStyleColor();
            //}
            //else {
                //Ref<Texture> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
                //hasPlayButton = ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5);
            //}

            if (hasPlayButton) {
                Ref<Texture> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
                ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
                if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5)) {
                    if (m_sceneState == SceneState::Edit)
                        OnScenePlay();
                    else if (m_sceneState == SceneState::Play)
                        OnSceneStop();
                }
                /*if (Engine::GetGameState() == GameState::Paused) {

                    {
                        if (Engine::GetGameState() == GameState::Stopped && GameState::Playing == GameState::Playing)
                            Engine::EnterPlayMode();
                        else if (GameState::Playing == GameState::Stopped)
                            Engine::ExitPlayMode();

                        Engine::SetGameState(GameState::Playing);
                    }

                    Color color = Engine::GetProject()->Settings.PrimaryColor;
                    std::string message = ICON_FA_RUNNING + std::string(" Playing...");
                    m_sceneHierarchyPanel.SetStatusMessage(message.c_str(), color);
                }
                else {

                    //SceneSnapshot->FullPath = currentScenePath;
                    SceneSnapshot = Scene::Copy(m_editorScene);

                    std::string statusMessage = ICON_FA_HAMMER + std::string("  Building .Net solution...");
                    m_sceneHierarchyPanel.SetStatusMessage(statusMessage);

                    m_sceneHierarchyPanel.SetContext(SceneSnapshot);

                    auto job = [this]()
                        {
                            auto project = Engine::GetProject();
                            auto& scriptingEngine = ScriptingEngineNet::Get();
                            const std::string& assemblyPath = scriptingEngine.GetGameAssemblyPath(project);
                            if (FileSystem::FileExists(assemblyPath))
                            {
                                errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                                FileSystem::GetFile(assemblyPath)->SetHasBeenModified(false);
                            }
                        };

                    JobSystem::Get().Dispatch(job, [this]()
                        {
                            bool containsError = false;
                            std::find_if(errors.begin(), errors.end(), [](const CompilationError& error) {
                                return error.isWarning == false;
                                });

                            if (errors.size() > 0 && containsError)
                            {
                                m_sceneHierarchyPanel.SetStatusMessage("Failed to build scripts! See Logger for more info", { 1.0f, 0.1f, 0.1f, 1.0f });

                                //Logger::Log("Build FAILED.", ".net", CRITICAL);
                                HZ_CORE_ERROR(".net * Build Failed");
                                for (CompilationError error : errors)
                                {
                                    const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                                    //Logger::Log(errorMessage, ".net", CRITICAL);
                                    HZ_CORE_ERROR(".net * {0}", errorMessage);
                                }
                            }
                            else
                            {
                                Engine::GetProject()->ExportEntitiesToTrenchbroom();

                                ImGui::SetWindowFocus("Logger");
                                m_sceneHierarchyPanel.SetStatusMessage("Entering play mode...");

                                Engine::SetGameState(GameState::Playing);
                                std::string statusMessage = ICON_FA_RUNNING + std::string(" Playing...");
                                m_sceneHierarchyPanel.SetStatusMessage(statusMessage.c_str(), Engine::GetProject()->Settings.PrimaryColor);
                            }
                        });
                }*/
            //}

            // Tooltip 
        }

        /*ImGui::SameLine();

        const bool wasPlayMode = Engine::GetGameState() != GameState::Stopped;
        if (!wasPlayMode)
            ImGui::BeginDisabled();

        if ((ImGui::ImageButton((ImTextureID)m_iconStop->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5))) {

            Engine::ExitPlayMode();

            SceneSnapshot = m_editorScene;
            Engine::SetCurrentScene(SceneSnapshot);

            m_sceneHierarchyPanel.SetContext(SceneSnapshot);
            m_sceneHierarchyPanel.SetStatusMessage("Ready");
        }

        if (!wasPlayMode)
            ImGui::EndDisabled();
        else {
            if (ImGui::BeginItemTooltip()) {

                ImGui::Text("Stop game [F5]");
                ImGui::EndTooltip();
            }
        }*/

        if (hasSimulateButton) {
        
            if (hasPlayButton)
                ImGui::SameLine();
        
            Ref<Texture> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play) ? m_iconSimulate : m_iconStop;
            if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0))) {
                if (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play)
                    OnSceneSimulate();
                else if (m_sceneState == SceneState::Simulate)
                    OnSceneStop();
            }
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::UpdateInputs()
    {
        // Input Keys
        bool control = Input::KeyDown(Key::LeftControl) || Input::KeyPressed(Key::RightControl);
        bool shift = Input::KeyDown(Key::LeftShift) || Input::KeyPressed(Key::RightShift);

        if (control) {
            if (Input::KeyPressed(Key::N)) NewScene();
            if (Input::KeyPressed(Key::O)) OpenScene();
            if (shift) { if (Input::KeyPressed(Key::S)) SaveAsScene(); }
            if (Input::KeyPressed(Key::S)) SaveScene();
            if (Input::KeyPressed(Key::D)) OnDuplicateEntity();
            if (Input::KeyPressed(Key::T)) ShaderManager::RebuildShaders();
        }
        if (Input::KeyPressed(Key::Delete)) {
            if (GImGui->ActiveId == 0) {
                Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
                if (selectedEntity) {
                    m_sceneHierarchyPanel.SetSelectedEntity({});
                    Engine::GetCurrentScene()->DestroyEntity(selectedEntity);
                }
            }
        }

        if (_viewportFocused && _viewportHovered) {
            if (Input::KeyPressed(Key::W)) m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
            if (Input::KeyPressed(Key::E)) m_gizmoType = ImGuizmo::OPERATION::ROTATE;
            if (Input::KeyPressed(Key::S)) m_gizmoType = ImGuizmo::OPERATION::SCALE;
        }

        if (Input::LeftMousePressed())
            if (_viewportHovered && !ImGuizmo::IsOver() && !Input::KeyDown(Key::LeftAlt))
                m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity);

        if (Engine::IsPlayMode()) {
            bool mouseShow = false;
            if (Input::KeyPressed(Key::Escape)) {
                if (!mouseShow) {
                    Input::ShowCursor();
                    mouseShow = true;
                }
                else {
                    Input::DisableCursor();
                    mouseShow = false;
                }
            }
        }
    }

    void EditorLayer::NewScene() {
        Ref<Scene> newScene = CreateRef<Scene>();
        m_sceneHierarchyPanel.SetContext(newScene);
        Engine::SetCurrentScene(newScene);
    }

    void EditorLayer::NewProject() {

        if (Engine::GetProject() && Engine::GetProject()->FileExist())
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

        Ref<Project> project = Project::New(String::Split(fileName, '.')[0], "Your Description", finalPath);
        Engine::LoadProject(project);
        Engine::SetCurrentScene(Scene::New());
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

        FileSystem::Scan();
        //std::cout << FileSystem::Root << "\n";
    }

    void EditorLayer::OpenScene() {

        std::string filepath = FileDialogs::OpenFile("Faint Scene (*.scene)\0*.scene\0");
        if (!filepath.empty()) {
            OpenScene(filepath);
        }
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path) {

        if (m_sceneState != SceneState::Edit)
            OnSceneStop();

        if (path.extension().string() != ".scene") {
            HZ_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        std::string finalPath = String::Split(path.string(), '.')[0];

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string())) {
            
            m_editorScene = newScene;
            m_sceneHierarchyPanel.SetContext(m_editorScene);
            currentScenePath = path.string();
            
            //m_activeScene = m_editorScene;
            Engine::SetCurrentScene(m_editorScene);
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!currentScenePath.empty()) {
            SceneSerializer serializer(Engine::GetCurrentScene());
            serializer.Serialize(currentScenePath);
        }
        else {
            SaveAsScene();
        }
        m_sceneHierarchyPanel.SetStatusMessage("Saved :D");
    }

    void EditorLayer::SaveAsScene() {
        std::string filepath = FileDialogs::SaveFile("Faint Scene (*.scene)\0*.scene\0");
        if (!filepath.empty()) {
            SceneSerializer serializer(Engine::GetCurrentScene());

            if (!String::EndsWith(filepath, ".scene"))
                filepath += ".scene";
            serializer.Serialize(filepath);
        }
    }

    void EditorLayer::OnScenePlay() {

        if (m_sceneState == SceneState::Simulate)
            OnSceneStop();

        m_sceneState = SceneState::Play;

        SceneSnapshot = Scene::Copy(m_editorScene);
        Engine::SetCurrentScene(SceneSnapshot);
        Engine::EnterPlayMode();
        
        std::string statusMessage = ICON_FA_HAMMER + std::string("  Building .Net solution...");
        m_sceneHierarchyPanel.SetStatusMessage(statusMessage);
        m_sceneHierarchyPanel.SetContext(SceneSnapshot);

        auto job = [this]()
            {
                auto project = Engine::GetProject();
                auto& scriptingEngine = ScriptingEngineNet::Get();
                const std::string& assemblyPath = scriptingEngine.GetGameAssemblyPath(project);
                if (FileSystem::FileExists(assemblyPath))
                {
                    errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                    FileSystem::GetFile(assemblyPath)->SetHasBeenModified(false);
                }
            };

        JobSystem::Get().Dispatch(job, [this]()
            {
                bool containsError = false;
                std::find_if(errors.begin(), errors.end(), [](const CompilationError& error) {
                    return error.isWarning == false;
                    });

                if (errors.size() > 0 && containsError)
                {
                    m_sceneHierarchyPanel.SetStatusMessage("Failed to build scripts! See Logger for more info", { 1.0f, 0.1f, 0.1f, 1.0f });

                    //Logger::Log("Build FAILED.", ".net", CRITICAL);
                    HZ_CORE_ERROR(".net - Build Failed");
                    for (CompilationError error : errors)
                    {
                        const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                        //Logger::Log(errorMessage, ".net", CRITICAL);
                        HZ_CORE_ERROR(".net - {0}", errorMessage);
                    }
                }
                else
                {
                    Engine::GetProject()->ExportEntitiesToTrenchbroom();

                    ImGui::SetWindowFocus("Console");
                    m_sceneHierarchyPanel.SetStatusMessage("Entering play mode...");
                    
                    //Input::DisableCursor();

                    Engine::SetGameState(GameState::Playing);
                    std::string statusMessage = ICON_FA_RUNNING + std::string(" Playing...");
                    m_sceneHierarchyPanel.SetStatusMessage(statusMessage.c_str(), Engine::GetProject()->Settings.PrimaryColor);
                }
            });
    }

    void EditorLayer::OnSceneSimulate() {

        if (m_sceneState == SceneState::Play)
            OnSceneStop();

        m_sceneState = SceneState::Simulate;
        SceneSnapshot = Scene::Copy(m_editorScene);
        Engine::SetCurrentScene(SceneSnapshot);
        Engine::EnterPlayMode();

        auto job = [this]()
            {
                auto project = Engine::GetProject();
                auto& scriptingEngine = ScriptingEngineNet::Get();
                const std::string& assemblyPath = scriptingEngine.GetGameAssemblyPath(project);
                if (FileSystem::FileExists(assemblyPath))
                {
                    errors = ScriptingEngineNet::Get().BuildProjectAssembly(Engine::GetProject());
                    FileSystem::GetFile(assemblyPath)->SetHasBeenModified(false);
                }
            };

        JobSystem::Get().Dispatch(job, [this]()
            {
                bool containsError = false;
                std::find_if(errors.begin(), errors.end(), [](const CompilationError& error) {
                    return error.isWarning == false;
                    });

                if (errors.size() > 0 && containsError)
                {
                    m_sceneHierarchyPanel.SetStatusMessage("Failed to build scripts! See Logger for more info", { 1.0f, 0.1f, 0.1f, 1.0f });

                    HZ_CORE_ERROR(".net - Build Failed");
                    for (CompilationError error : errors)
                    {
                        const std::string errorMessage = error.file + "( line " + std::to_string(error.line) + "): " + error.message;
                        HZ_CORE_ERROR(".net - {0}", errorMessage);
                    }
                }
                else
                {
                    Engine::GetProject()->ExportEntitiesToTrenchbroom();

                    ImGui::SetWindowFocus("Console");
                    m_sceneHierarchyPanel.SetStatusMessage("Entering play mode...");

                    Input::DisableCursor();

                    Engine::SetGameState(GameState::Playing);
                    std::string statusMessage = ICON_FA_RUNNING + std::string(" Playing...");
                    m_sceneHierarchyPanel.SetStatusMessage(statusMessage.c_str(), Engine::GetProject()->Settings.PrimaryColor);
                }
            });
    }

    void EditorLayer::OnSceneStop() {

        HZ_CORE_ASSERT(m_sceneState == SceneState::Play || m_sceneState == SceneState::Simulate);
        if (m_sceneState == SceneState::Play)
            Engine::ExitPlayMode();

        m_sceneState = SceneState::Edit;
        //m_activeScene->OnRuntimeStop();
        SceneSnapshot = m_editorScene;
        Engine::SetCurrentScene(SceneSnapshot);

        m_sceneHierarchyPanel.SetContext(SceneSnapshot);
        m_sceneHierarchyPanel.SetStatusMessage("Ready");
    }

    void EditorLayer::OnDuplicateEntity() {

        if (m_sceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
            m_editorScene->DuplicateEntity(selectedEntity);
    }
}