#pragma once

#include "Core/Base.h"
#include "Core/Time.h"
#include "Core/Window.h"
#include "AssetManagment/Project.h"

namespace Faint {

	class Project;
	class Scene;
	class EngineSubsystem;
	class EngineSubsystemScriptable;
	
	namespace Physics {
		class PhysicsEngine;
	}

	enum GameState {
		Loading,
		Playing,
		Paused,
		Stopped,
	};

	class Engine {
	public:
		static void Init();  // Initialize the engine.
		static void Update(Time time);// Updates everything, called every frame.
		static void Close(); // Shutdown the engine. 

		static void EnterPlayMode(); // Start the game
		static void ExitPlayMode(); // Start the game

		static void Draw(); // Start new frame
		static void EndDraw(); // Swap buffer

		static void SetGameState(GameState state) { gameState = state; }
		static GameState GetGameState() { return gameState; }
		static bool IsPlayMode() { return gameState == GameState::Playing; }
		static float SetTime(float t) { return time += t; }
		static float GetTime() { return time; }

		static bool LoadProject(Ref<Project> project);
		static Ref<Project> GetProject();

		// Scene
		static bool SetCurrentScene(Ref<Scene> scene);
		static bool QueueSceneSwitch(const std::string& scene);
		static Ref<Scene> GetCurrentScene();

		static bool LoadScene(Ref<Scene> scene);
		static Ref<Scene> LoadScene(const std::string& path);

		// Window
		static Ref<Window> GetCurrentWindow();

		static Ref<EngineSubsystemScriptable> GetScriptedSubsystem(const std::string& subsystemName);
		static Ref<EngineSubsystemScriptable> GetScriptedSubsystem(const int subsystemId);

	protected:
		static void OnWindowSetScene(Ref<Scene> oldScene, Ref<Scene> newScene);
		
		static void OnScriptingEngineGameAssemblyLoaded();

		static void OnScenePreInitialize(Ref<Scene> scene);
		static void OnScenePostInitialize(Ref<Scene> scene);

	private:
		static Ref<Project> currentProject;
		static Ref<Scene> currentScene;
		static Ref<Window> currentWindow;
		static std::string queuedScene;
		static float time;

		static inline std::vector<Ref<EngineSubsystem>> subsystems;
		static inline std::unordered_map<std::string, Ref<EngineSubsystemScriptable>> scriptedSubsystemMap;

		//static Ref<Physics::PhysicsEngine> m_physicsEngine;

		static GameState gameState;
	};
}