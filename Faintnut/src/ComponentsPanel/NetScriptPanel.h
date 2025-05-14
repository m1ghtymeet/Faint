#pragma once
#include "Core/Base.h"
#include "ComponentPanel.h"

class NetScriptPanel : ComponentPanel {
private:
	bool _expanded = false;

	std::string _importedPathMesh;
public:
	NetScriptPanel() = default;

	void Draw(Faint::Entity& entity);
};