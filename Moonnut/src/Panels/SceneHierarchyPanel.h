#pragma once

#include "Math/Math.h"
#include "Debug/Log.h"
#include "Scene/Scene.h"
#include "../Core/EditorActions.h"
#include "../Core/PanelsManager.h"

#include <imgui.h>

enum class TransformSpace {
	LOCAL,
	GLOBAL
};

class SceneHierarchyPanel {
public:
	SceneHierarchyPanel(const std::string& p_title, bool p_opened, EditorActions& p_editorActions, PanelsManager& p_panelsManager);

	/**
	* Select the widget corresponding to the given gameobject
	* @param p_gb
	*/
	//void SelectEntityByInstance(Entity& p_gb);

	void SetScene(Moon::Scene* p_scene);

	void SetTransformSpace(TransformSpace& space);

	void Draw();

	Moon::Entity* GetSelectedEntity() const;
	void SetSelectedEntity(Moon::Entity* entity);

private:
	void DrawEntityHeader(Moon::Entity* entity);
	void DrawTransformComponent(Moon::Entity* entity);
	void DrawEntityNode(Moon::Entity* entity);
	void DrawEntityCreationMenu(Moon::Entity* parentEntity = nullptr);
	void DrawAllComponents(Moon::Entity* entity);
	void DrawAllBehaviours(Moon::Entity* entity);

	ImGuiTreeNodeFlags GetEntityNodeFlags(Moon::Entity* entity);
	template<typename T>
	void DisplayAddComponentEntry(const std::string& entryName);
private:
	TransformSpace m_transformSpace;
	EditorActions& m_editorActions;
	PanelsManager& m_panelsManager;
private:
	Moon::Scene* m_scene = nullptr;
	Moon::Entity* _selectionContext = nullptr;
	std::unordered_set<int64_t> m_expandedNodes;
	std::string searchQuery = "";

	// Status bar
	std::string m_StatusMessage = "";
	glm::vec4 m_StatusBarColor = glm::vec4(0.08f, 0.08f, 0.08f, 1.0f);
};