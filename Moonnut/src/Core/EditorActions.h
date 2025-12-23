#pragma once
#include <future>
#include <Event/Event.h>
#include "../Renderer/Types.h"
#include "Context.h"

class EditorActions {
public:
	EditorActions(Context& p_context);
	
	virtual ~EditorActions();

	Context& GetContext();

	void StartPlaying();

	void PauseGame();

	void StopPlaying();

	Moon::Entity& CreateEmptyEntity(Moon::Entity* p_parent = nullptr, const std::string& p_name = "", const std::string& p_tag = "");

	Moon::Entity& CreateEntityWithModel(const std::string& p_path, Moon::Entity* p_parent, const std::string& p_name);

	bool DestroyEntity(Moon::Entity& p_entity);

	void DuplicateEntity(Moon::Entity& p_toDuplicate, Moon::Entity* p_forcedParent);

	void SaveEntityAsPrefab(Moon::Entity& p_entity);

	enum class EEditorMode { EDIT, PLAY, PAUSE };

	struct HandleEvent {
		glm::vec3 startPosition;
		glm::vec3 endPosition;
		glm::vec3 delta;
		int handleIndex = -1;
		bool active = false;
	};
	using HandleDragCallback = std::function<void(const HandleEvent&)>;

	EEditorMode GetEditorMode() const;

	void SetEditorMode(EEditorMode p_newEditorMode);

	void SetHandleDragCallback(const HandleDragCallback& cb);

	const HandleEvent& GetHandleEvent() const;

	void ProcessHandleEvents(const ICamera& camera, ColliderGizmoState& colliderGizmo, int mouseX, int mouseY, int viewportWidth, int viewportHeight);

	#pragma region SCENE
	void LoadEmptyScene();

	void LoadSceneFromDisk(const std::string& p_path, bool p_absolute = false);

	void SaveSceneToDisk(Moon::Scene& p_scene, const std::string& p_path);

	void SaveSceneChanges();

	void SaveAsScene();
	#pragma endregion

	#pragma region BUILDING
	void Build(bool p_autoRun = false, bool p_temoFolder = false);

	void BuildAtLocation(const std::string& p_configuration, const std::filesystem::path& p_buildPath, bool p_autoRun = false);
	#pragma endregion

public:
	Moon::Event<EEditorMode> EditorModeChangedEvent;
	Moon::Event<> PlayEvent;

private:
	Context& m_context;

	EEditorMode m_editorMode = EEditorMode::EDIT;

	HandleEvent m_handleEvent;
	HandleDragCallback m_handleDragCallback;
	//std::unique_ptr<Moon::Scene> m_sceneBackup;
	std::unique_ptr<json> m_sceneBackup;
};