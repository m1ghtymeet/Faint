#pragma once

#include "Core/Base.h"
#include "Math/Math.h"
#include "Debug/Log.h"
#include "Scene/Scene.h"

class SceneHierarchyPanel {
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(const std::string& p_title, bool p_opened);

	/**
	* Select the widget corresponding to the given gameobject
	* @param p_gb
	*/
	//void SelectEntityByInstance(Entity& p_gb);

	void Draw();

	Faint::Entity* GetSelectedEntity() const;
	void SetSelectedEntity(Faint::Entity* entity);

	void SetStatusMessage(const std::string& message, Faint::Color color = Faint::Color{ 0.1f, 0.1f, 0.1f, 1 });

	void DrawStatusBar();
public:
	//Event<Entity&> EntitySelectedEvent;
	//Event<Entity&> EntityUnselectedEvent;
private:
	void DrawComponents(Faint::Entity* entity);
	template<typename T>
	void DisplayAddComponentEntry(const std::string& entryName);
private:
	Faint::Entity* _selectionContext = nullptr;
	Faint::Entity* QueueDeletion = nullptr;
	std::string searchQuery = "";
	bool m_IsRenaming = false;

	// Status bar
	std::string m_StatusMessage = "";
	Faint::Color m_StatusBarColor = Faint::Color(0.08f, 0.08f, 0.08f, 1.0f);
};