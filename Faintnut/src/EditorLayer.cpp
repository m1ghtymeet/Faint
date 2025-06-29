#include "EditorLayer.h"

#include "Input/Input.h"

#include "Renderer/SceneRenderer.h"
#include "UI/ImUI.h"

#include "Threading/JobSystem.h"

#include "Util/PlatformUtil.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Math/Math.h"
#include "AssetManagment/Config.h"
#include "Engine.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <TextEditor.h>

namespace Faint {

    extern const std::filesystem::path s_assetPath;
    std::string currentScenePath = "";
    bool projectSettingsVisible = false;

    // Test
    TextEditor editor;

    EditorLayer::EditorLayer() : Layer("EditorLayer") {
    }

    void EditorLayer::OnAttach() {
        HZ_PROFILE_FUNCTION();
        
        m_iconPlay = CreateRef<Texture>("data/editor/icons/PlayButton.png");
        m_iconStop = CreateRef<Texture>("data/editor/icons/StopButton.png");
        m_iconSimulate = CreateRef<Texture>("data/editor/icons/SimulateButton.png");

        auto project = Project::New();
        try {
            Engine::LoadProject(project);
        } catch (std::exception exception) {
            HZ_CORE_ERROR("Error Loading project!");
        }

        Engine::Init();
        Engine::GetCurrentWindow()->SetTitle("Faint Editor");
        Engine::GetCurrentWindow()->SetIcon("data/editor/icons/faint-logo.png");
        
        m_editorScene = CreateRef<Scene>();
        Engine::SetCurrentScene(m_editorScene);

        m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;

        // Windows
        _NewProjectWindow = new NewProjectWindow();

        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
        editor.SetPalette(TextEditor::GetDarkPalette());
        
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig iconsConfigBold;

        UI::NormalBold = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 30);
        UI::Subtitle = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/ClearSans-Bold.ttf", 40);
        UI::IconSolid = ImGui::GetIO().Fonts->AddFontFromFileTTF("data/editor/fonts/fa-solid-900.ttf", 11, 0, icon_ranges);
    }

    void EditorLayer::OnDetach() {
        HZ_PROFILE_FUNCTION();
    }

    void EditorLayer::OnUpdate(Time time) {
        HZ_PROFILE_FUNCTION();

        if (_viewportSize.x != 0 && _viewportSize.y != 0)
            Engine::GetCurrentScene()->OnViewportResize((float)_viewportSize.x, (float)_viewportSize.y);

        Engine::Update(time);
        Physics::SubmitDebugLinesToRenderer(DebugRenderMode::PHYSX_ALL);
        Engine::Draw();

        // Update scene
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_viewportBounds[0].x;
        my -= m_viewportBounds[0].y;
        glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (!Engine::IsPlayMode() && m_sceneViewportHovered && !ImGuizmo::IsUsing()
            && mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            bool foundSomethingToSelect = false;
            //auto* gBuffer = SceneRenderer::GetFrameBuffer("GBuffer");
            //if (!foundSomethingToSelect) {
            //    gBuffer->Bind();
            //    glReadBuffer(gBuffer->GetColorAttachmentSlotByName("MousePick"));
            //    int result;
            //    glReadPixels((int)mouseX, (int)mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &result);
            //    {
            //        //m_hoveredEntity = result == 0 ? Entity() : Entity(result - 1, "", "", 1);
            //        SceneRenderer::m_hoveredEntityID = result;
            //        foundSomethingToSelect = true;
            //    }
            //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //}
        }
        OnImGuiRender();

        // Resize
        if (Window::Get()->viewportWidth != _viewportSize.x || Window::Get()->viewportHeight != _viewportSize.y) {
            Window::Get()->viewportWidth = _viewportSize.x;
            Window::Get()->viewportHeight = _viewportSize.y;

            mINI::INIFile file = Config::Begin("config/layout.ini", true);
            std::stringstream ss;
            ss << Window::Get()->viewportWidth << "," << Window::Get()->viewportHeight;
            Config::currentINI["Viewport"]["Size"] = ss.str();
            file.write(Config::currentINI);
        }

        Engine::EndDraw();

        Input::Update();
        UpdateInputs();
    }

    void EditorLayer::OnImGuiRender() {

        HZ_PROFILE_FUNCTION();

        static bool dockSpaceOpen = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
        }
        else {
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
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (!_NewProjectWindow->HasCreatedProject()) {
            _NewProjectWindow->Draw();
            if (Engine::GetProject()->DefaultScene->FullPath != "")
                OpenScene(Engine::GetProject()->DefaultScene->FullPath);
        }

        style.WindowMinSize.x = minWinSize;

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    NewProject();
                }
                if (ImGui::MenuItem("Open...")) {
                    OpenProject();
                }
                if (ImGui::MenuItem("Save...")) {
                    
                }
                if (ImGui::MenuItem("Save As...")) {

                }
                ImGui::Separator();
                if (ImGui::MenuItem("Set current scene as default")) {
                    Engine::GetProject()->DefaultScene = Engine::GetCurrentScene();
                }
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    NewScene();
                }
                if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
                    OpenScene();
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    SaveScene();
                }
                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                    SaveAsScene();
                }

                if (ImGui::MenuItem("Exit")) Faint::Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Project Settings")) {
                    projectSettingsVisible = !projectSettingsVisible;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Build")) {
                if (ImGui::MenuItem("Build")) {
                    std::string buildPath = FileDialogs::SaveFile("Build path");
                    if (buildPath != "") {
                        bool failed = false;
                        HZ_CORE_INFO("Preparing to build at location: \"" + buildPath + "\"");
                        std::filesystem::remove_all(buildPath);
                        if (std::filesystem::create_directory(buildPath)) {
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

            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Build Shaders", "Ctrl+T"))
                    SceneRenderer::LoadShaders();

                if (ImGui::MenuItem("Generate Visual Solution", NULL)) {
                    ScriptingEngineNet::Get().GenerateSolution(FileSystem::Root, Engine::GetProject()->Name);
                    m_sceneHierarchyPanel.SetStatusMessage("Visual studio solution generated succesfully.");
                }
#ifdef FT_DEBUG
                if (ImGui::MenuItem("Copy Faint.NET", NULL)) {
                    ScriptingEngineNet::Get().CopyFaintNETAssemblies(FileSystem::Root);
                    m_sceneHierarchyPanel.SetStatusMessage("Faint.Net assemblies succesfully copied.");
                }
#endif
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();

        m_sceneHierarchyPanel.Draw();
        if (_NewProjectWindow->HasCreatedProject())
            m_sceneHierarchyPanel.DrawStatusBar();
        m_fileBrowserPanel.Draw();
        m_projectSettingsPanel.Draw(projectSettingsVisible);
        //m_hardwareInfo.Draw();

        //editor.Render("Text Editor", ImVec2(800, 600));

        // Logger
        if (ImGui::Begin("Console")) {
            // Toolbar
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
                if (ImGui::Button("Clear", ImVec2(60, 28))) {
                    Log::ClearLogs();
                    m_sceneHierarchyPanel.SetStatusMessage("Clear Logs!");
                }

                ImGui::SameLine();

                if (ImGui::Button(ICON_FA_FILTER, ImVec2(30, 28))) {
                    ImGui::OpenPopup("filter_popup");
                }

                bool isEnabled = true;
                if (ImGui::BeginPopup("filter_popup")) {
                    ImGui::SeparatorText("Filters");
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

                    if (isEnabled) {
                        glm::vec4 color = Engine::GetProject()->Settings.PrimaryColor;
                        ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
                    }

                    if (ImGui::Button((std::string(ICON_FA_BAN) + " Error").c_str())) {
                        // Add filter logic
                    }

                    if (isEnabled)
                        ImGui::PopStyleColor();

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(2);
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar(2);
            }

            ImGui::Separator();

            // Log Table
            if (ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
                for (auto& l : Log::GetLogs()) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));

                    ImGui::BeginGroup();

                    // Time
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 255, 150, 255));
                    ImGui::Text("[%s]", l.time.c_str());
                    ImGui::PopStyleColor();

                    ImGui::SameLine();

                    // Log Name
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(98, 174, 239, 255));
                    ImGui::Text(l.log.c_str());
                    ImGui::PopStyleColor();

                    ImGui::SameLine();

                    // Log Message
                    if (l.type == "trace")
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
                    else if (l.type == "error") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 0.8f));
                    }
                    else if (l.type == "warn") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.1f, 0.8f));
                    }
                    else if (l.type == "info") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
                    }
                    std::string displayMessage = l.message;
                    if (l.count > 0)
                        displayMessage += "(" + std::to_string(l.count) + ")";
                    ImGui::TextWrapped("%s", displayMessage.c_str());
                    ImGui::PopStyleColor();

                    ImGui::EndGroup();
                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Scene View");
        {
            ImGuizmo::BeginFrame();
            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_sceneViewportFocused = ImGui::IsWindowFocused();
            m_sceneViewportHovered = ImGui::IsWindowHovered();

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

            if (_viewportSize != *((glm::vec2*)&viewportPanelSize)) {
                _viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
                Engine::GetCurrentScene()->OnViewportResize(_viewportSize.x, _viewportSize.y);
            }
            uint32_t textureID = SceneRenderer::GetFrameBuffer("FinalImage")->GetColorAttachmentHandleByName("Color");
            ImGui::Image((void*)textureID, ImVec2{ _viewportSize.x, _viewportSize.y }, ImVec2{0, 1}, ImVec2{1, 0});

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Scene")) {
                    char* path = (char*)payload->Data;
                    OpenScene(path);
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model")) {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    fullPath = String::ReplaceSlash(fullPath);

                    Entity& entity = Engine::GetCurrentScene()->CreateEntity(FileSystem::GetFileNameFromPath(fullPath));
                    MeshRendererComponent& component = entity.AddComponent<MeshRendererComponent>();
                    Model* model = AssetManager::LoadModel(fullPath, true);
                    component.SetModel(model);

                    entity.GetComponent<TransformComponent>()->SetLocalPosition(glm::vec3(0, 0, 0));
                }
                ImGui::EndDragDropTarget();
            }

            Engine::GetCurrentScene()->selectedType = NONE;

            // Gizmos
            Entity* selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
            if (!selectedEntity) SceneRenderer::m_selectedEntityID = 0;
            else SceneRenderer::m_selectedEntityID = selectedEntity->GetID();
            if (selectedEntity && m_gizmoType != -1) {
                //if (selectedEntity->GetComponent<CameraComponent>())
                //    Engine::GetCurrentScene()->selectedType = _CAMERA;
                //if (selectedEntity->GetComponent<LightComponent>())
                //    Engine::GetCurrentScene()->selectedType = _LIGHT;

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
                auto* tc = selectedEntity->GetComponent<TransformComponent>();
                glm::mat4 transform = tc->GetGlobalMatrix();
                glm::vec3 originalRotation = Vec3(tc->GetGlobalRotation().x, tc->GetGlobalRotation().y, tc->GetGlobalRotation().z);

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

                    if (selectedEntity->HasParent()) {
                        const auto& parentTransformComponent = selectedEntity->GetParent()->GetComponent<TransformComponent>();
                        const Matrix4& parentTransform = parentTransformComponent->GetGlobalMatrix();
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

                    tc->SetLocalPosition(localPosition);

                    if (m_gizmoType != ImGuizmo::SCALE)
                        tc->SetLocalRotation(localRotation);

                    tc->SetLocalScale(localScale);
                    tc->SetLocalMatrix(newLocalTransform);
                }
            }
        }
        ImGui::End();
        
        ImGui::Begin("Game View");
        {
            m_gameViewportFocused = ImGui::IsWindowFocused();
            m_gameViewportHovered = ImGui::IsWindowHovered();
            ImVec2 gameViewportSize = ImGui::GetContentRegionAvail();
            if (m_gameViewportSize != *(glm::vec2*)&gameViewportSize) {
                m_gameViewportSize = { gameViewportSize.x, gameViewportSize.y };
            }
            //uint32_t textureID = SceneRenderer::GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("Lighting");
            //ImGui::Image((void*)textureID, gameViewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
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

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

        float size = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

        bool hasPlayButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play;
        bool hasSimulateButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate;
        bool hasPauseButton = m_sceneState != SceneState::Edit;

        if (hasPlayButton) {
            Ref<Texture> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
            ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0) || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5)) {
                if (m_sceneState == SceneState::Edit)
                    OnScenePlay();
                else if (m_sceneState == SceneState::Play)
                    OnSceneStop();
            }
        }

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
            if (Input::KeyPressed(Key::T)) SceneRenderer::LoadShaders();
        }
        if (Input::KeyPressed(Key::Delete)) {
            if (GImGui->ActiveId == 0) {
                Entity* selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
                if (selectedEntity) {
                    m_sceneHierarchyPanel.SetSelectedEntity(nullptr);
                    //Engine::GetCurrentScene()->DestroyEntity(selectedEntity);
                }
            }
        }

        if (m_sceneViewportFocused && m_sceneViewportHovered) {
            if (Input::KeyPressed(Key::W) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
            if (Input::KeyPressed(Key::E) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::ROTATE;
            if (Input::KeyPressed(Key::S) && !Input::KeyDown(Key::LeftControl)) m_gizmoType = ImGuizmo::OPERATION::SCALE;
        }

        if (Input::LeftMousePressed())
            if (m_sceneViewportHovered && !ImGuizmo::IsOver() && !Input::KeyDown(Key::LeftAlt))
                m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity);

        if (Engine::IsPlayMode()) {
            bool mouseShow = false;
            if (Input::KeyPressed(Key::Escape)) {
                if (!mouseShow) {
                    //Input::ShowCursor();
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
        Engine::SetCurrentScene(CreateRef<Scene>());
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
        m_sceneHierarchyPanel.SetSelectedEntity(nullptr);

        Ref<Scene> newScene = CreateRef<Scene>();
        std::string content = FileSystem::ReadFile(path.string(), true);
        Engine::GetCurrentScene()->GetAllEntities().clear();
        json j = json::parse(content);
        Engine::GetCurrentScene()->Deserialize(j);
    }

    void EditorLayer::SaveScene() {
        Engine::GetProject()->SerializeYaml(Engine::GetProject()->FullPath);

        if (!currentScenePath.empty()) {
            //SceneSerializer serializer(Engine::GetCurrentScene());
            //serializer.Serialize(currentScenePath);
            json j = Engine::GetCurrentScene()->Serialize();
            std::string out = j.dump(4);
            std::ofstream fout(Engine::GetCurrentScene()->FullPath);
            fout << out.c_str();
        }
        else {
            SaveAsScene();
        }
        auto view = Engine::GetCurrentScene()->Reg().view<MeshRendererComponent>();
        for (auto e : view) {
            auto meshRenderer = view.get<MeshRendererComponent>(e);
            for (auto& mesh : meshRenderer.GetModel()->GetMeshes()) {
                std::string fileData = mesh->GetMaterial()->Serialize().dump(4);
                FileSystem::BeginWriteFile(mesh->GetMaterial()->Path, true);
                FileSystem::WriteLine(fileData);
                FileSystem::EndWriteFile();
            }
        }
        m_sceneHierarchyPanel.SetStatusMessage("Saved!");
    }

    void EditorLayer::SaveAsScene() {
        std::string filepath = FileDialogs::SaveFile("Faint Scene (*.scene)\0*.scene\0");
        if (!filepath.empty()) {
            //SceneSerializer serializer(Engine::GetCurrentScene());

            if (!String::EndsWith(filepath, ".scene"))
                filepath += ".scene";
            //serializer.Serialize(filepath);
        }
    }

    void EditorLayer::OnScenePlay() {

        if (m_sceneState == SceneState::Simulate)
            OnSceneStop();

        m_sceneState = SceneState::Play;

        SceneSnapshot = m_editorScene;
        Engine::SetCurrentScene(SceneSnapshot);
        Engine::EnterPlayMode();
        
        std::string statusMessage = ICON_FA_HAMMER + std::string("  Building .Net solution...");
        m_sceneHierarchyPanel.SetStatusMessage(statusMessage);

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
                    Log::ClearLogs();
                }
            });
    }

    void EditorLayer::OnSceneSimulate() {

        if (m_sceneState == SceneState::Play)
            OnSceneStop();

        m_sceneState = SceneState::Simulate;
        //SceneSnapshot = Scene::Copy(m_editorScene);
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
        SceneSnapshot = m_editorScene;
        Engine::SetCurrentScene(SceneSnapshot);

        m_sceneHierarchyPanel.SetStatusMessage("Ready");
    }

    void EditorLayer::OnDuplicateEntity() {
        //if (m_sceneState != SceneState::Edit)
        //    return;
        if (Engine::IsPlayMode())
            return;

        Entity* selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
        //if (selectedEntity)
        //    m_editorScene->DuplicateEntity(*selectedEntity);
    }
}