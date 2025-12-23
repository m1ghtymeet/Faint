#include "Engine.h"
#include <Debug/Instrumentor.h>

#include "Physics/Physics.h"

#include "FileSystem/FileSystem.h"
//#include "Scripting/ScriptingEngineNet.h"
#include "Scene/Scene.h"
#include "Input/Input.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Moon {

    Ref<Project> Engine::currentProject;
    Ref<Scene> Engine::currentScene;
    //Ref<Window> Engine::currentWindow;
    std::string Engine::queuedScene = "";

    GameState Engine::gameState = GameState::Stopped;
    float Engine::time = 0;

    float fixedUpdateRate = 1.0f / 90.0f;
    float fixedUpdateDifference = 0.0f;

    void Engine::Init() {
        //ScriptingEngineNet::Get().OnGameAssemblyLoaded().AddStatic(&Engine::OnScriptingEngineGameAssemblyLoaded);

        //Physics::Init();

        // Create a window
        //currentWindow = CreateRef<Window>(Settings::WindowSettings());

        HZ_CORE_INFO("Engine Initialized!");
    }

    void Engine::Update(float p_deltaTime) {
        //JobSystem::Get().Update();

        if (Engine::IsPlayMode()) {
            if (!queuedScene.empty()) {
                Ref<Scene> nextScene = CreateRef<Scene>();
                if (FileSystem::FileExists(queuedScene)) {
                    const std::string& fileContent = FileSystem::ReadFile(queuedScene);

                    //GetCurrentScene()->OnExit();

                    //SetCurrentScene(nextScene);

                    //PhysicsManager::Get().ReInit();
                    //GetCurrentScene()->OnInit();

                    queuedScene = "";
                }
            }
        }

        float scaledDeltatime = p_deltaTime * 1.0f;

        //if (GetCurrentScene()) {

            //GetCurrentScene()->Update(scaledDeltatime);

            //if (!Engine::IsPlayMode())
            //    GetCurrentScene()->EditorUpdate(p_deltaTime);

            fixedUpdateDifference += p_deltaTime;

            while (fixedUpdateDifference >= fixedUpdateRate) {
                fixedUpdateDifference -= fixedUpdateRate;
            }

        //}
    }

    void Engine::Close() {

    }

    void Engine::EndDraw() {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    bool Engine::LoadProject(Ref<Project> project) {
        currentProject = project;
        /*if (!Engine::SetCurrentScene(currentProject->defaultScene))
            return false;*/
        //FileSystem::SetRootDirectory(FileSystem::GetParentPath(project->FullPath));
        //ScriptingEngineNet::Get().Uninitialize();
        //ScriptingEngineNet::Get().Initialize();
        //ScriptingEngineNet::Get().LoadProjectAssembly(project);
        return true;
    }

    bool Engine::SetCurrentScene(Ref<Scene> scene) {
        currentScene = scene;
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
        return currentScene;
    }

    //Ref<Window> Engine::GetCurrentWindow() {
    //    return currentWindow;
    //}

    bool Engine::LoadScene(Ref<Scene> scene) {
       
        return true;
    }

    Ref<Scene> Engine::LoadScene(const std::string& path)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        //SceneSerializer serializer(scene);
        //serializer.Deserialize(path);
        return scene;
    }

    void Engine::OnScriptingEngineGameAssemblyLoaded() {

        if (!Engine::IsPlayMode())
            return;

        //subsystems.clear();
        //scriptedSubsystemMap.clear();

        //const Coral::ManagedAssembly& gameAssembly = ScriptingEngineNet::Get().GetGameAssembly();

        //const auto scriptTypeEngineSubsystem = gameAssembly.GetType("Moon.Net.EngineSubsystem");
        
        //const auto& types = gameAssembly.GetTypes();
        //for (const auto& type : types) {
        //
        //    // Initialize all subsystems
        //    if (type->IsSubclassOf(scriptTypeEngineSubsystem))
        //    {
        //        const std::string typeName = std::string(type->GetFullName());
        //        HZ_CORE_INFO("Creating Scripted Subsystem {0}", typeName);
        //
        //        Coral::ManagedObject scriptedSubsystem = type->CreateInstance();
        //        scriptedSubsystem.SetPropertyValue("EngineSubsystemID", subsystems.size());
        //        Ref<EngineSubsystemScriptable> subsystemScript = CreateRef<EngineSubsystemScriptable>(scriptedSubsystem);
        //        subsystems.push_back(subsystemScript);
        //
        //        scriptedSubsystemMap[typeName] = subsystemScript;
        //
        //        subsystemScript->Initialize();
        //    }
        //}
    }
    void Engine::OnScenePreInitialize(Ref<Scene> scene)
    {
    }
}
