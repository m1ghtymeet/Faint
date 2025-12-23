#include "EditorActions.h"
#include <Core/String.h>
#include <Core/GlobalLocator.h>
#include <Engine.h>
#include <Debug/Assertion.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/MaultFile.h>
#include <Input/Input.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <Math/Ray.h>
#include <Util/PlatformUtil.h>
#include <fstream>
#include <iostream>

extern ColliderGizmoState g_colliderGizmo;

EditorActions::EditorActions(Context& p_context) :
		m_context(p_context)
{
}

EditorActions::~EditorActions()
{
}

Context& EditorActions::GetContext() {
	return m_context;
}

void EditorActions::StartPlaying() {
	if (m_editorMode == EEditorMode::EDIT) {
        m_context.scriptEngine->Reload();

        if (m_context.scriptEngine->IsOk()) {
            PlayEvent.Invoke();

            HZ_CORE_INFO("Is Now Playing...");
            m_context.sceneManager.GetCurrentScene()->Play();

            m_sceneBackup = std::make_unique<json>(m_context.sceneManager.GetCurrentScene()->Serialize());
            SetEditorMode(EEditorMode::PLAY);
		}
        else {
            HZ_CORE_ERROR("Cannot play the scene, script engine has errors.");
            Moon::MessageBox message(
                "Script Engine Error",
                "Cannot play the scene, script engine has errors.\nCheck the console for more information.",
                Moon::MessageBox::EMessageType::ERROR
            );
        }
	}
	else {
		SetEditorMode(EEditorMode::PLAY);
	}
}

void EditorActions::PauseGame() {
	SetEditorMode(EEditorMode::PAUSE);
}

void EditorActions::StopPlaying() {
	
    if (m_editorMode != EEditorMode::EDIT) {
        Moon::Input::ShowCursor();
        SetEditorMode(EEditorMode::EDIT);
        bool loadedFromDisk = m_context.sceneManager.IsCurrentSceneLoadedFromPath();
        std::string sceneSourcePath = m_context.sceneManager.GetCurrentSceneSourcePath();

        int64_t focusedActorID = -1;

        FT_CORE_ASSERT(m_sceneBackup.operator bool(), "No Scene backup to restore");
        // To be able to save or reload the scen whereas the scene was loaded from disk, we need to restore the scene from the backup
        m_context.sceneManager.LoadSceneFromMemory(*m_sceneBackup);
        if (loadedFromDisk)
            m_context.sceneManager.StoreCurrentSceneSourcePath(sceneSourcePath);
        m_sceneBackup.reset();
    }
}

Moon::Entity& EditorActions::CreateEmptyEntity(Moon::Entity* p_parent, const std::string& p_name, const std::string& p_tag) {
    const auto currentScene = m_context.sceneManager.GetCurrentScene();
    auto& instance = p_name.empty() ? currentScene->CreateEntity() : currentScene->CreateEntity(p_name, p_tag);

    if (p_parent)
        instance.SetParent(*p_parent);

    HZ_CORE_INFO("Entity '" + instance.GetName() + "' created!");

    return instance;
}

Moon::Entity& EditorActions::CreateEntityWithModel(const std::string& p_path, Moon::Entity* p_parent, const std::string& p_name) {
    const auto currentScene = m_context.sceneManager.GetCurrentScene();
    Moon::Entity& entity = m_context.sceneManager.GetCurrentScene()->CreateEntity(p_name);
    entity.AddComponent<Moon::MeshRendererComponent>();
    auto& comp = entity.AddComponent<Moon::MeshRendererComponent>();
    Moon::Model* model = FTSERVICE(Moon::AssetManagment::ModelManager).LoadResource(p_path);
    comp.SetModel(model);
    comp.ModelPath = p_path;

    Moon::MaterialRendererComp& materialComp = entity.AddComponent<Moon::MaterialRendererComp>();
    Moon::Rendering::Material* material = Moon::Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
    materialComp.AddMaterialByIndex(0, *material);
    return entity;
}

bool EditorActions::DestroyEntity(Moon::Entity& p_entity) {
    HZ_CORE_INFO("Entity destroyed");
    return m_context.sceneManager.GetCurrentScene()->DestroyEntity(p_entity);
}

void EditorActions::DuplicateEntity(Moon::Entity& p_toDuplicate, Moon::Entity* p_forcedParent) {
    const auto j = p_toDuplicate.Serialize();
    auto& newEntity = CreateEmptyEntity(nullptr, p_toDuplicate.GetName(), p_toDuplicate.GetTag());
    int64_t idToUse = newEntity.GetID();
    newEntity.Deserialize(j);

    newEntity.SetID(idToUse);
}

void EditorActions::SaveEntityAsPrefab(Moon::Entity& p_entity) {
	// Save to disk
    std::string savePath = Moon::FileDialogs::SaveFile("Prefab (*.prefab)\0*.prefab\0");
    if (savePath.empty())
        return;

    if (savePath.size() < 7 || savePath.substr(savePath.size() < 7) != ".prefab")
        savePath += ".prefab";

    json entityJson = p_entity.Serialize();
    { // Serialize entity and children
        entityJson["Name"] = p_entity.GetName();
        entityJson["ID"] = p_entity.GetID();

        if (!p_entity.GetChildren().empty()) {
            json childrenArray = json::array();
            for (Moon::Entity* child : p_entity.GetChildren()) {
                json childJson = child->Serialize();
                childJson["Name"] = child->GetName();
                childJson["ID"] = child->GetID();
                {
                    if (!child->GetChildren().empty()) {
                        json grandChildrenArray = json::array();
                        for (Moon::Entity* grandChild : child->GetChildren()) {
                            json grandChildJson = grandChild->Serialize();
                            grandChildJson["Name"] = grandChild->GetName();
                            grandChildJson["ID"] = grandChild->GetID();
                            grandChildrenArray.push_back(grandChildJson);
                        }
                        childJson["Children"] = grandChildrenArray;
                    }
                    childrenArray.push_back(childJson);
                }
            }
            entityJson["Children"] = childrenArray;
        }

        if (entityJson.contains("ParentID"))
            entityJson.erase("ParentID");
    }

    try {
        std::ofstream file(savePath);
        file << std::setw(4) << entityJson;
        file.close();
        HZ_CORE_INFO("Entity saved as Prefab: " + savePath);
    }
    catch (const std::exception& e) {
        HZ_CORE_ERROR("Failed to save prefab: " + std::string(e.what()));
    }
}
EditorActions::EEditorMode EditorActions::GetEditorMode() const {
    return m_editorMode;
}

void EditorActions::SetEditorMode(EEditorMode p_newEditorMode) {
	m_editorMode = p_newEditorMode;
}

void EditorActions::SetHandleDragCallback(const HandleDragCallback& cb) {
    m_handleDragCallback = cb;
}

const EditorActions::HandleEvent& EditorActions::GetHandleEvent() const {
    return m_handleEvent;
}

void EditorActions::ProcessHandleEvents(const ICamera& camera, ColliderGizmoState& colliderGizmo, int mouseX, int mouseY, int viewportWidth, int viewportHeight) {
    if (m_editorMode != EEditorMode::EDIT)
        return;

    bool leftPressed = Moon::Input::LeftMousePressed();

    Ray ray = Moon::Math::GetMouseRay(mouseX, mouseY, viewportWidth, viewportHeight, camera);
    colliderGizmo.hoveredHandle = -1;
    float closestT = FLT_MAX;
    for (int i = 0; i < 8; i++) {
        float t;
        if (Moon::Math::RaySphereIntersection(ray, colliderGizmo.handlePositions[i], 0.1f, t)) {
			if (t < closestT) {
				closestT = t;
				colliderGizmo.hoveredHandle = i;
			}
        }
    }

    if (leftPressed)
        colliderGizmo.selectedHandle = colliderGizmo.hoveredHandle;
    else if (!Moon::Input::LeftMouseDown())
        colliderGizmo.selectedHandle = -1;

    //std::cout << "Hovered: " << colliderGizmo.hoveredHandle << ", Selected: " << colliderGizmo.selectedHandle << "\n";
}

void EditorActions::LoadEmptyScene() {

    if (GetEditorMode() != EEditorMode::EDIT)
        StopPlaying();

    m_context.sceneManager.LoadDefaultScene();

    HZ_CORE_INFO("New scene created");
}

void EditorActions::SaveSceneToDisk(Moon::Scene& p_scene, const std::string& p_path) {
	json j = p_scene.Serialize();
	std::string out = j.dump(4);
	std::ofstream fout(p_path);
	fout << out.c_str();
}

void EditorActions::SaveSceneChanges() {
    auto currentScene = m_context.sceneManager.GetCurrentScene();
    FT_CORE_ASSERT(currentScene, "Current scene is null!");

    std::string sceneSourcePath = m_context.sceneManager.GetCurrentSceneSourcePath();
    if (sceneSourcePath.empty()) {
        HZ_CORE_WARN("Scene source path is empty. Using Save As instead.");
        SaveAsScene();
        return;
    }
    const std::string currentScenePath = Moon::FileSystem::Root + sceneSourcePath + ".scene";
    if (!Moon::FileSystem::FileExists(currentScenePath, true)) {
        HZ_CORE_ERROR("Failed to save scene: {}", currentScenePath);
        return;
    }
    SaveSceneToDisk(*currentScene, currentScenePath);
    currentScene->FullPath = currentScenePath;
    Moon::Engine::GetProject()->defaultScene = currentScene;
    Moon::Engine::GetProject()->Save();
    HZ_CORE_INFO("Current scene saved to: {}", currentScenePath);
}

void EditorActions::SaveAsScene() {
	std::string filepath = Moon::FileDialogs::SaveFile("Scene (*.scene)\0*.scene\0");
	if (!filepath.empty()) {
		if (!Moon::String::EndsWith(filepath, ".scene"))
			filepath += ".scene";
	
		auto currentScene = m_context.sceneManager.GetCurrentScene();
		SaveSceneToDisk(*currentScene, filepath);
		HZ_CORE_INFO("Current scene saved to: " + filepath);
	}
}

void EditorActions::Build(bool p_autoRun, bool p_temoFolder) {
    
    
}

void EditorActions::BuildAtLocation(const std::string& p_configuration, const std::filesystem::path& p_buildPath, bool p_autoRun) {
    std::string executableName = Moon::Engine::GetProject()->Name + ".exe";
    bool failed = false;

    HZ_CORE_INFO(std::format("Preparing to build at location: \"{}\"", p_buildPath.string()));
    std::filesystem::remove_all(p_buildPath);
    std::filesystem::create_directories(p_buildPath / "data" / "game");

    auto copyDir = [&](const std::string& src, const std::string& dst) {
        std::error_code err;
        std::filesystem::copy(src, p_buildPath / dst, std::filesystem::copy_options::recursive, err);
        if (err) { failed = true; HZ_CORE_ERROR("Copy failed: {}", err.message()); }
        else HZ_CORE_INFO("Copied: {} -> {}", src, dst);
    };

    copyDir(Moon::FileSystem::Root + Moon::Engine::GetProject()->Name + ".project", "data/game/Example.project");
    copyDir("data/shaders", "data/shaders");
    copyDir("data/engine", "data/engine");
    copyDir(Moon::FileSystem::Root + "Scripts", "data/game/Scripts");

    // --- Copy cache if exists ---
    if (Moon::FileSystem::DirectoryExists(Moon::FileSystem::Root + "Cache", true)) {
        copyDir(Moon::FileSystem::Root + "Cache", "data/game/Cache");
    }

    std::filesystem::path assetsRoot = Moon::FileSystem::Root + "Assets";
    if (std::filesystem::exists(assetsRoot)) {
        //for (const auto& entry : std::filesystem::directory_iterator(assetsRoot)) {
        //    std::string name = entry.path().filename().string();
        //    std::string maultName = name + ".mault";
        //    std::string maultPath = (p_buildPath / "data" / "game" / maultName).string();
        //
        //    HZ_CORE_INFO("Compressing {} -> {}", name, maultPath);
        //
        //    if (entry.is_directory()) {
        //        if (!Moon::FileSystem::CompressDirectoryToZip(entry.path().string(), maultPath)) {
        //            HZ_CORE_ERROR("Failed to create mault for directory: {}", name);
        //            failed = true;
        //        }
        //    }
        //    else {
        //        if (!Moon::FileSystem::CompressFileToZip(entry.path().string(), maultPath)) {
        //            HZ_CORE_ERROR("Failed to create mault for file: {}", name);
        //            failed = true;
        //        }
        //    }
        //}
        {
            std::filesystem::path destAssets = p_buildPath / "data" / "game" / "Assets";
            std::error_code err;
            std::filesystem::copy(assetsRoot, destAssets, std::filesystem::copy_options::recursive, err);
            if (err) {
                failed = true;
                HZ_CORE_ERROR("Failed to copy Assets: {} -> {} | {}", assetsRoot.string(), destAssets.string(), err.message());
            }
            else {
                HZ_CORE_INFO("Copied entire Assets folder: {} -> {}", assetsRoot.string(), destAssets.string());
            }
        }
    }

    // --- Copy builder executables ---
    std::string builderFolder = "builder/" + p_configuration + "/";
    if (std::filesystem::exists(builderFolder)) {
        std::filesystem::copy(builderFolder, p_buildPath, std::filesystem::copy_options::recursive);
        std::error_code err;
        std::filesystem::rename(p_buildPath / "MoonGame.exe", p_buildPath / executableName, err);
        if (err) failed = true;
    }

    if (failed) {
        std::filesystem::remove_all(p_buildPath);
        HZ_CORE_ERROR("An error occured during the building of your game.\nCheck the console for more information");
        Moon::MessageBox message(
            "Build Failure",
            "An error occured during the building of your game.\nCheck the console for more information",
            Moon::MessageBox::EMessageType::ERROR
        );
    }
    else {
        HZ_CORE_INFO("Game built successfully at " + p_buildPath.string());
        Moon::MessageBox message(
            "Build Successfully",
            "Your Game Build At " + p_buildPath.string()
        );
    }
}
