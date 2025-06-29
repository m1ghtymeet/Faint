#include "ModelResourceInspector.h"
#include <imgui/imgui.h>
#include "MaterialEditor.h"

namespace Faint {
	Ref<Material> selectedMaterial = nullptr;
	bool openMaterialEditor = false;

	ModelResourceInspector::ModelResourceInspector(Ref<Faint::Model> model) {
		_model = model;
	}
	ModelResourceInspector::ModelResourceInspector(Ref<Faint::SkinnedModel> model) {
		_skinnedModel = model;
	}
	void ModelResourceInspector::Draw() {

		ImGui::BeginChild("modelInspector", ImVec2(0, 300 * std::size(_model->GetMeshes())), true);
		{
			for (uint32_t i = 0; i < std::size(_model->GetMeshes()); i++) {
				std::string headerLabel = "Mesh " + _model->GetMeshes()[i]->GetName();
				if (ImGui::CollapsingHeader(headerLabel.c_str())) {
					Ref<Material> material = _model->GetMeshes()[i]->GetMaterial();
					std::string materialName = material->Path.empty() ? "" : FileSystem::GetFileNameFromPath(material->Path);
					ImGui::Text("Material: %s", _model->GetMeshes()[i]->GetMaterial()->GetName().c_str());

					std::string buttonName = "Material Undefined";
					if (material && !materialName.empty())
						buttonName = materialName;
					if (ImGui::Button(buttonName.c_str())) {
						std::string file = FileDialogs::OpenFile("Material File (*.material)\0*.material\0");
						if (!file.empty()) {
							Ref<Material> newMaterial = AssetManager::LoadMaterial(FileSystem::AbsoluteToRelative(file));
							if (newMaterial) {
								_model->GetMeshes()[i]->SetMaterial(newMaterial);
							}
						}
					}
					if (ImGui::Button(("Edit##" + std::to_string(i)).c_str())) {
						selectedMaterial = material;
						openMaterialEditor = true;
					}
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Material")) {
							char* path = (char*)payload->Data;
							std::string fullPath = std::string(path, 512);
					
							Ref<Material> newMaterial = AssetManager::LoadMaterial(FileSystem::AbsoluteToRelative(fullPath));
							if (newMaterial) {
								_model->GetMeshes().at(i)->SetMaterial(newMaterial);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			}
		}
		ImGui::EndChild();
		
		if (openMaterialEditor && selectedMaterial) {
			ImGui::Begin("Material Editor", &openMaterialEditor, ImGuiWindowFlags_AlwaysAutoResize);
			MaterialEditor editor;
			editor.Draw(selectedMaterial);
			ImGui::End();
		}
	}
}