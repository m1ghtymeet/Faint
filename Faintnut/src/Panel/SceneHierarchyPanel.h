#pragma once

#include "Core/Base.h"
#include "Debug/Log.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"

#include "Panel.h"

class SceneHierarchyPanel : public Panel {
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(const Ref<Scene>& scene);

	void SetContext(const Ref<Scene>& scene);

	virtual void OnImGuiRender() override;

	Entity GetSelectedEntity() const { return _selectionContext; }
	void SetSelectedEntity(Entity entity);

	void SetStatusMessage(const std::string& message, Color color = Color{ 0.1f, 0.1f, 0.1f, 1 }) {
		m_StatusMessage = message;
		m_StatusBarColor = color;
	}

	void DrawStatusBar();
private:
	void DrawGameObjectNode(Entity entity);
	void DrawComponents(Entity& entity);
	void DrawEntityTree(Entity entity, bool drawChildren = true);

	template<typename T>
	void DisplayAddComponentEntry(const std::string& entryName);
private:
	Ref<Scene> _context;
	Entity _selectionContext;
	Entity QueueDeletion;
	std::string searchQuery = "";
	bool m_IsRenaming = false;

	// Status bar
	std::string m_StatusMessage = "";
	Color m_StatusBarColor = Color(0.08f, 0.08f, 0.08f, 1.0f);

	friend class Scene;
};