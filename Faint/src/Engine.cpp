#include "Engine.h"

#include "Core/Audio.h"

#include "Physics/Physics.h"

#include "FileSystem/FileSystem.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/SceneRenderer.h"
#include "Scripting/ScriptingEngineNet.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Input/Input.h"

#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/EngineSubsystemScriptable.h"

#include "Threading/JobSystem.h"
#include "Modules/Modules.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Faint {

    Ref<Project> Engine::currentProject;
    Ref<Scene> Engine::currentScene;
    Ref<Window> Engine::currentWindow;
    std::string Engine::queuedScene = "";

    GameState Engine::gameState = GameState::Stopped;
    float Engine::time = 0;

    float fixedUpdateRate = 1.0f / 90.0f;
    float fixedUpdateDifference = 0.0f;

    void Engine::Init() {

        ScriptingEngineNet::Get().OnGameAssemblyLoaded().AddStatic(&Engine::OnScriptingEngineGameAssemblyLoaded);

        AudioManager::Get().Initialize();
        //PhysicsManager::Get().Init();
        Physics::Init();

        // Create a window
        currentWindow = Window::Get();
        SceneRenderer::Init();

        Input::Init();
        Renderer2D::Init();
        HZ_CORE_INFO("Engine Initialized!");

        Modules::StartupModules();
    }

    void Engine::Update(Time time)
    {
        JobSystem::Get().Update();

        if (Engine::IsPlayMode()) {
            if (!queuedScene.empty()) {
                Ref<Scene> nextScene = Scene::New();
                if (FileSystem::FileExists(queuedScene)) {
                    const std::string& fileContent = FileSystem::ReadFile(queuedScene);
                    SceneSerializer serializer(nextScene);
                    serializer.Deserialize(queuedScene);

                    GetCurrentScene()->OnExit();

                    SetCurrentScene(nextScene);

                    //PhysicsManager::Get().ReInit();
                    GetCurrentScene()->OnInit();

                    queuedScene = "";
                }
            }
        }

        float scaledDeltatime = time * 1.0f;

        if (Engine::IsPlayMode()) {

            for (auto subsystem : subsystems)
            {
                if (subsystem == nullptr)
                    continue;

                if (subsystem->CanEverTick())
                    subsystem->Update(scaledDeltatime);
            }
        }

        if (currentWindow->GetScene()) {

            currentWindow->Update(scaledDeltatime);

            if (!Engine::IsPlayMode())
                GetCurrentScene()->EditorUpdate(time);

            fixedUpdateDifference += time;

            while (fixedUpdateDifference >= fixedUpdateRate) {
                currentWindow->FixedUpdate(fixedUpdateRate);
                //Physics::StepPhysics(fixedUpdateRate);
                fixedUpdateDifference -= fixedUpdateRate;
            }

            Input::Update();
            AudioManager::Get().AudioUpdate();
        }
    }

    void Engine::EnterPlayMode()
    {
        if (GetGameState() == GameState::Playing || GetGameState() == GameState::Loading) {
            HZ_CORE_ERROR("Cannot enter play mode if is already in play mode or is loading.");
            return;
        }

        SetGameState(GameState::Loading);

        //PhysicsManager::Get().ReInit();

        if (GetCurrentScene()->OnInit()) {
            SetGameState(GameState::Playing);
        }
        else {
            HZ_CORE_ERROR("Cannot enter play mode. Scene OnInit failed!");
            GetCurrentScene()->OnExit();
        }
    }

    void Engine::ExitPlayMode()
    {
        // Don't trigger exit if already not in play mode
        if (GetGameState() != GameState::Stopped) {
            GetCurrentScene()->OnExit();
            Input::ShowCursor();
            SetGameState(GameState::Stopped);
        }
    }

    void Engine::Draw()
    {
        // ZoneScoped

        RenderCommand::Clear();

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        Window::Get()->Draw();
    }

    void Engine::EndDraw()
    {
        Window::Get()->EndDraw();
    }

    bool Engine::LoadProject(Ref<Project> project) {

        currentProject = project;
        /*if (!Engine::SetCurrentScene(currentProject->defaultScene))
            return false;*/

        FileSystem::SetRootDirectory(FileSystem::GetParentPath(project->FullPath));
        ScriptingEngineNet::Get().Uninitialize();
        ScriptingEngineNet::Get().Initialize();
        ScriptingEngineNet::Get().LoadProjectAssembly(project);

        return true;
    }

    bool Engine::SetCurrentScene(Ref<Scene> scene) {

        currentWindow->SetScene(scene);

        return true;
    }

    bool Engine::QueueSceneSwitch(const std::string& scene) {

        if (!Engine::IsPlayMode()) {
            return false;
        }

        queuedScene = scene;
        return true;
    }

    Ref<Project> Engine::GetProject() {
        return currentProject;
    }
    Ref<Scene> Engine::GetCurrentScene() {

        if (currentWindow)
            return currentWindow->GetScene();
        return nullptr;
    }

    Ref<Window> Engine::GetCurrentWindow()
    {
        return currentWindow;
    }

    bool Engine::LoadScene(Ref<Scene> scene)
    {
        currentWindow->SetScene(scene);
        //SceneSerializer serializer(scene);
        //return serializer.Deserialize(scene->FullPath);
        return true;
    }

    Ref<Scene> Engine::LoadScene(const std::string& path)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(path);
        return scene;
    }

    Ref<EngineSubsystemScriptable> Engine::GetScriptedSubsystem(const std::string& subsystemName)
    {
        if (scriptedSubsystemMap.contains(subsystemName))
            return scriptedSubsystemMap[subsystemName];

        return nullptr;
    }

    Ref<EngineSubsystemScriptable> Engine::GetScriptedSubsystem(const int subsystemId)
    {
        if (subsystemId >= subsystems.size()) {
            return nullptr;
        }
        return std::reinterpret_pointer_cast<EngineSubsystemScriptable>(subsystems[subsystemId]);
    }

    void Engine::OnScriptingEngineGameAssemblyLoaded() {

        if (!Engine::IsPlayMode())
            return;

        subsystems.clear();
        scriptedSubsystemMap.clear();

        const Coral::ManagedAssembly& gameAssembly = ScriptingEngineNet::Get().GetGameAssembly();

        const auto scriptTypeEngineSubsystem = gameAssembly.GetType("Faint.Net.EngineSubsystem");
        
        const auto& types = gameAssembly.GetTypes();
        for (const auto& type : types) {

            // Initialize all subsystems
            if (type->IsSubclassOf(scriptTypeEngineSubsystem))
            {
                const std::string typeName = std::string(type->GetFullName());
                HZ_CORE_INFO("Creating Scripted Subsystem {0}", typeName);

                Coral::ManagedObject scriptedSubsystem = type->CreateInstance();
                scriptedSubsystem.SetPropertyValue("EngineSubsystemID", subsystems.size());
                Ref<EngineSubsystemScriptable> subsystemScript = CreateRef<EngineSubsystemScriptable>(scriptedSubsystem);
                subsystems.push_back(subsystemScript);

                scriptedSubsystemMap[typeName] = subsystemScript;

                subsystemScript->Initialize();
            }
        }
    }
    void Engine::OnScenePreInitialize(Ref<Scene> scene)
    {
    }
}
