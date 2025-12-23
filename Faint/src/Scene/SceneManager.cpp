#include "SceneManager.h"
#include <FileSystem/FileSystem.h>
#include <Debug/Log.h>

Moon::SceneManager::SceneManager(const std::string& p_sceneRootFolder) : m_sceneRootFolder(p_sceneRootFolder){
	LoadEmptyScene();
}

Moon::SceneManager::~SceneManager() {
	UnloadCurrentScene();
}

void Moon::SceneManager::Update() {
	if (m_delayedLoadCall) {
		m_delayedLoadCall();
		m_delayedLoadCall = 0;
	}
}

void Moon::SceneManager::LoadEmptyScene() {
	UnloadCurrentScene();
	m_currentScene.reset(new Scene());
	SceneLoadEvent.Invoke();
}

void Moon::SceneManager::LoadDefaultScene() {
	UnloadCurrentScene();
	m_currentScene.reset(new Scene());
	m_currentScene->AddDefaultCamera();
	SceneLoadEvent.Invoke();
}

bool Moon::SceneManager::LoadScene(const std::string& p_path, bool p_absolute) {

	std::filesystem::path path = p_absolute ? FileSystem::Root : std::filesystem::path{ m_sceneRootFolder };
	
	path /= p_path;
	path += ".scene";
	HZ_CORE_TRACE("Loaded Scene: {}", path.string());

	if (FileSystem::FileExists(path.string(), true)) {
		std::string contentFile = FileSystem::ReadFile(path.string(), true);
		json js = json::parse(contentFile);
		if (LoadSceneFromMemory(js)) {
			StoreCurrentSceneSourcePath(p_path);
			return true;
		}
	}

	return false;
}

bool Moon::SceneManager::LoadSceneFromMemory(json& p_data) {
	LoadEmptyScene();
	m_currentScene->Deserialize(p_data);
	return true;
}

void Moon::SceneManager::UnloadCurrentScene() {
	if (m_currentScene) {
		m_currentScene.reset();
		SceneUnloadEvent.Invoke();
	}
}

void Moon::SceneManager::SetCurrentScene(std::unique_ptr<Scene> p_scene) {
	UnloadCurrentScene();
	m_currentScene = std::move(p_scene);
}

bool Moon::SceneManager::HasCurrentScene() const {
	return m_currentScene != nullptr;
}

Moon::Scene* Moon::SceneManager::GetCurrentScene() const {
	return m_currentScene.get();
}

std::string Moon::SceneManager::GetCurrentSceneSourcePath() const {
	return m_currentSceneSourcePath;
}

bool Moon::SceneManager::IsCurrentSceneLoadedFromPath() const {
	return m_currentSceneLoadedFromPath;
}

void Moon::SceneManager::StoreCurrentSceneSourcePath(const std::string& p_path) {

	m_currentSceneSourcePath = p_path;
	m_currentSceneLoadedFromPath = true;
	CurrentSceneSourcePathChangedEvent.Invoke(m_currentSceneSourcePath);
}
