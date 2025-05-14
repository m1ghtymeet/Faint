#pragma once
#include "Core/Base.h"
#include "ComponentPanel.h"

class MeshPanel : ComponentPanel {
private:
	bool _expanded = false;

	std::string _importedPathMesh;
public:
	MeshPanel() {
		
	}

	void Draw(Faint::Entity& entity) {

		using namespace Faint;

		BeginComponentTable(MESH, MeshRendererComponent);
		{
			ImGui::Text("Model");
			ImGui::TableNextColumn();


		}
		
	}
};