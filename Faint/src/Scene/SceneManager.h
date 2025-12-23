#pragma once

#include <Scene/Scene.h>

namespace Moon {
	class SceneManager {
	public:
		SceneManager(const std::string& p_sceneRootFolder = "");

		~SceneManager();

		void Update();

		void LoadEmptyScene();

		void LoadDefaultScene();

		bool LoadScene(const std::string& p_path, bool p_absolute = false);

		bool LoadSceneFromMemory(json& p_data);

		void UnloadCurrentScene();

		void SetCurrentScene(std::unique_ptr<Scene> p_scene);

		bool HasCurrentScene() const;

		Scene* GetCurrentScene() const;
		
		std::string GetCurrentSceneSourcePath() const;

		bool IsCurrentSceneLoadedFromPath() const;

		void StoreCurrentSceneSourcePath(const std::string& p_path);

	public:
		Event<> SceneLoadEvent;
		Event<> SceneUnloadEvent;
		Event<const std::string&> CurrentSceneSourcePathChangedEvent;

	private:
		const std::string m_sceneRootFolder;
		std::unique_ptr<Scene> m_currentScene = nullptr;

		bool m_currentSceneLoadedFromPath = false;
		std::string m_currentSceneSourcePath = "";

		std::function<void()> m_delayedLoadCall;
	};
}