#include "SceneHierarchyPanel.h"
#include "Core/String.h"
#include "Core/OS.h"
#include "Util/PlatformUtil.h"
#include <Core/GlobalLocator.h>
#include <FileSystem/FileSystem.h>
#include <Scripting/ScriptTypes.h>

#include "UI/ImUI.h"
#include "../Helpers/PopupHelper.h"
#include "../Helpers/ImGuiHelper.h"
#include "../Panels/MaterialEditor.h"

// Resources
#include <AssetManagment/MeshManager.h>
#include <AssetManagment/Loader/ModelLoader.h>
#include <AssetManagment/Loader/TextureLoader.h>
#include <AssetManagment/Loader/MaterialLoader.h>
#include <FontAwesome5.h>

#include <imgui/imgui_stdlib.h>
#include <sol/sol.hpp>

// Components
#include "Scene/Components.h"

using namespace Moon;

SceneHierarchyPanel::SceneHierarchyPanel(const std::string& p_title, bool p_opened, EditorActions& p_editorActions, PanelsManager& p_panelsManager) :
	m_editorActions(p_editorActions),
	m_panelsManager(p_panelsManager)
{
	
}

//void SceneHierarchyPanel::SelectEntityByInstance(Entity& p_gb) {
//	_selectionContext = &p_gb;
//}

void SceneHierarchyPanel::SetScene(Moon::Scene* p_scene) {
	m_scene = p_scene;
}

void SceneHierarchyPanel::SetTransformSpace(TransformSpace& space) {
	m_transformSpace = space;
}

void SceneHierarchyPanel::Draw() {
	if (!m_scene) return;

	if (ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse)) {
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.35f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.4f, 0.9f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));

		for (const auto& entity : m_scene->GetAllEntities()) {
			if (!entity->HasParent())
				DrawEntityNode(entity.get());
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(3);
		
		// Right click on empty space for adding new entity
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
		if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			DrawEntityCreationMenu(nullptr);
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();

	ImGui::Begin("Properties");
	if (_selectionContext) {
		DrawEntityHeader(_selectionContext);
		ImGui::Separator();

		DrawTransformComponent(_selectionContext);
		DrawAllComponents(_selectionContext);
		DrawAllBehaviours(_selectionContext);

		ImGui::Separator();
		ImGui::Spacing();

		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (200 / 2));
		if (ImGui::Button("Add Component", ImVec2(200, 0)))
			ImGui::OpenPopup("AddComponentPopup");

		if (ImGui::BeginPopup("AddComponentPopup")) {
			struct ComponentEntry {
				const char* name;
				std::function<void()> addFn;
			};

			std::vector<ComponentEntry> entries = {
				{ "Audio Source",		   [&]() { DisplayAddComponentEntry<AudioComponent>("Audio Source"); } },
				{ "Audio Listener",		   [&]() { DisplayAddComponentEntry<AudioListenerComponent>("Audio Listener"); } },
				{ "Box Collider",		   [&]() { DisplayAddComponentEntry<BoxColliderComponent>("Box Collider"); } },
				{ "Camera",				   [&]() { DisplayAddComponentEntry<CameraComponent>("Camera"); } },
				{ "Character Controller",  [&]() { DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller"); } },
				{ "Directional Light",	   [&]() { DisplayAddComponentEntry<DirectionalLightComp>("Directional Light"); } },
				{ "Mesh Renderer",		   [&]() { DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer"); } },
				{ "Skinned Mesh Renderer", [&]() { DisplayAddComponentEntry<SkinnedMeshRendererComp>("Skinned Mesh Renderer"); } },
				{ "Point Light",		   [&]() { DisplayAddComponentEntry<PointLightComponent>("Point Light"); } },
				{ "Rigid Body",			   [&]() { DisplayAddComponentEntry<RigidBodyComponent>("RigidBody"); } },
				{ "Text 2D",			   [&]() { DisplayAddComponentEntry<Text2DComponent>("Text2D"); } },
				{ "Widget",				   [&]() { DisplayAddComponentEntry<UI::WidgetComponent>("Widget"); } }
			};
			std::sort(entries.begin(), entries.end(),
				[](const ComponentEntry& a, const ComponentEntry& b) {
					return std::string(a.name) < std::string(b.name);
				});

			for (auto& e : entries) e.addFn();
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

bool IsAncestorOf(Moon::Entity* potentialParent, Moon::Entity* child) {
	Moon::Entity* current = child->GetParent();
	while (current) {
		if (current == potentialParent)
			return true;
		current = current->GetParent();
	}
	return false;
}

ImGuiTreeNodeFlags SceneHierarchyPanel::GetEntityNodeFlags(Moon::Entity* entity) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
		ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

	if (_selectionContext && _selectionContext->GetID() == entity->GetID())
		flags |= ImGuiTreeNodeFlags_Selected;

	if (entity->GetChildren().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (m_expandedNodes.count(entity->GetID()))
		flags |= ImGuiTreeNodeFlags_DefaultOpen;

	return flags;
}

void SceneHierarchyPanel::DrawEntityHeader(Moon::Entity* entity) {
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	strcpy_s(buffer, sizeof(buffer), entity->GetName().c_str());
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 80);
	if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
		entity->SetName(std::string(buffer));
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	bool active = entity->IsActive();
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	if (ImGui::Checkbox("##Active", &active))
		entity->SetActive(active);
	ImGui::PopStyleColor();

	char tagBuffer[256];
	strcpy_s(tagBuffer, entity->GetTag().c_str());
	ImGui::PushItemWidth(120);
	if (ImGui::InputTextWithHint("##Tag", "Empty Tag", tagBuffer, sizeof(tagBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
		entity->SetTag(std::string(tagBuffer));
	}
	ImGui::PopItemWidth();
}

void SceneHierarchyPanel::DrawTransformComponent(Moon::Entity* entity) {
	if (!entity->transform) return;

	auto* t = entity->transform;
	ImGui::PushID("Transform");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImGui::Separator();
	bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
		ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
		ImGuiTreeNodeFlags_FramePadding, "Transform");
	ImGui::PopStyleVar();

	if (open) {
		glm::vec3 position = t->GetLocalPosition();
		DrawVec3Control("Position", &position, 0.0f, 100.0f, false);
		t->SetLocalPosition(position);

		glm::quat rotation = t->GetLocalRotation();
		glm::vec3 euler = Moon::Math::QuaternionToEulerDegrees(rotation);
		DrawVec3Control("Rotation", &euler, 0.0f, 100.0f, false);
		t->SetLocalRotation(glm::quat(glm::radians(euler)));

		glm::vec3 scale = t->GetLocalScale();
		DrawVec3Control("Scale", &scale, 1.0f, 100.0f, true); // true = isScale
		t->SetLocalScale(scale);

		ImGui::TreePop();
	}
	ImGui::PopID();
}

std::string renameEntity = "";
void SceneHierarchyPanel::DrawEntityNode(Entity* entity) {
	ImGui::TableNextColumn();

	bool opened = ImGui::TreeNodeEx(
		(void*)(uint64_t)entity->GetID(),
		GetEntityNodeFlags(entity),
		entity->IsActive() ? entity->GetName().c_str() : (entity->GetName() + " (Deactivated)").c_str()
	);

	// ** Drag Source **
	if (ImGui::BeginDragDropSource()) {
		int64_t entityID = entity->GetID();
		ImGui::SetDragDropPayload("_ENTITY", &entityID, sizeof(int64_t));
		ImGui::Text("Moving %s", entity->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	// ** Drag Target **
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_ENTITY")) {
			int64_t droppedEntityID = *(const int64_t*)payload->Data;
			if (droppedEntityID != entity->GetID()) {
				Entity* droppedEntity = m_scene->GetEntityByID(droppedEntityID);
				if (droppedEntity && !IsAncestorOf(entity, droppedEntity)) {
					if (droppedEntity->HasParent()) {
						auto& siblings = droppedEntity->GetParent()->GetChildren();
						siblings.erase(std::remove(siblings.begin(), siblings.end(), droppedEntity), siblings.end());
					}
					droppedEntity->SetParent(*entity);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Selection
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		SetSelectedEntity(entity);

	// Right-click on entity
	if (ImGui::BeginPopupContextItem()) {
		// Existing entity actions
		if (ImGui::MenuItem("Duplicate Entity")) {
			m_editorActions.DuplicateEntity(*entity, nullptr);
		}
		if (ImGui::MenuItem("Remove Entity")) {
			m_editorActions.DestroyEntity(*entity);
			SetSelectedEntity(nullptr);
		}
		if (ImGui::BeginMenu("Rename Entity")) {
			ImGui::Text("Rename:");
			ImGui::SetNextItemWidth(160);

			if (ImGui::InputText("##Rename", &renameEntity, ImGuiInputTextFlags_EnterReturnsTrue)) {
				entity->SetName(renameEntity);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Unparent", nullptr, false, entity->HasParent())) {
			entity->DetachFromParent();
		}
		if (ImGui::MenuItem(entity->IsActive() ? "Deactivate" : "Activate")) {
			entity->SetActive(!entity->IsActive());
		}
		ImGui::Separator();

		// Add as Child submenu
		if (ImGui::BeginMenu("Add Child")) {
			DrawEntityCreationMenu(entity);
			ImGui::EndMenu();
		}

		ImGui::Separator();
		if (ImGui::MenuItem("Save as Prefab...")) {
			m_editorActions.SaveEntityAsPrefab(*entity);
		}
		ImGui::EndPopup();
	}

	if (opened) {
		for (Entity* child : entity->GetChildren())
			DrawEntityNode(child);
		ImGui::TreePop();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SCRIPT")) {
			std::string droppedPath = (const char*)payload->Data;
			std::string scriptName = FileSystem::GetFileNameFromPath(droppedPath);

			if (!entity->GetBehaviour(scriptName)) {
				entity->AddBehaviour(scriptName, droppedPath);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void SceneHierarchyPanel::DrawEntityCreationMenu(Moon::Entity* parentEntity) {
	// "Create Empty"
	if (ImGui::MenuItem(" Create Empty")) {
		Entity& newEntity = m_editorActions.CreateEmptyEntity();
		if (parentEntity)
			newEntity.SetParent(*parentEntity);
		SetSelectedEntity(parentEntity ? parentEntity : &newEntity);
	}

	// "3D Object" submenu
	if (ImGui::BeginMenu(" 3D Object")) {
		std::string folderPath = "data/engine/models/";
		bool hasAnyModel = false;

		try {
			for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
				if (!entry.is_regular_file()) continue;
				std::string path = entry.path().string();
				std::string ext = entry.path().extension().string();
				if (ext == ".obj" || ext == ".fbx") {
					std::string name = entry.path().stem().string();

					hasAnyModel = true;
					if (ImGui::MenuItem(name.c_str())) {
						Entity& newEntity = m_editorActions.CreateEntityWithModel(path, nullptr, name);
						if (parentEntity)
							newEntity.SetParent(*parentEntity);
						SetSelectedEntity(parentEntity ? parentEntity : &newEntity);
					}
				}
			}
			if (!hasAnyModel) {
				ImGui::MenuItem("(no models found)");
			}
		}
		catch (const std::exception& e) {
			ImGui::MenuItem("(Error reading models folder)");
		}
		ImGui::EndMenu();
	}

	// "Light" submenu
	if (ImGui::BeginMenu(" Light")) {
		if (ImGui::MenuItem("Point Light")) {
			Entity& newEntity = m_scene->CreateEntity("Point Light");
			newEntity.AddComponent<PointLightComponent>();
			if (parentEntity)
				newEntity.SetParent(*parentEntity);
			SetSelectedEntity(parentEntity ? parentEntity : &newEntity);
		}
		if (ImGui::MenuItem("Directional Light")) {
			Entity& newEntity = m_scene->CreateEntity("Directional Light");
			auto& comp = newEntity.AddComponent<DirectionalLightComp>();
			comp.SetStrength(2.0f);
			newEntity.transform->SetGlobalRotation(
				glm::angleAxis(glm::radians(-45.0f), glm::vec3(1, 0, 0)) *
				glm::angleAxis(glm::radians(35.0f), glm::vec3(0, 1, 0))
			);
			if (parentEntity)
				newEntity.SetParent(*parentEntity);
			SetSelectedEntity(parentEntity ? parentEntity : &newEntity);
		}
		ImGui::EndMenu();
	}
}

Entity* SceneHierarchyPanel::GetSelectedEntity() const {
	return _selectionContext;
}

void SceneHierarchyPanel::SetSelectedEntity(Entity* entity) {
	_selectionContext = entity;

	Entity* current = entity ? entity->GetParent() : nullptr;
	while (current) {
		m_expandedNodes.insert(current->GetID());
		current = current->GetParent();
	}
}

template<typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity* entity, UIFunction ui) {
	if(!entity->GetComponent<T>()) return;

	const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_Framed |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_AllowItemOverlap |
		ImGuiTreeNodeFlags_FramePadding;

	ImGui::PushID(name.c_str());

	auto* component = entity->GetComponent<T>();
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImGui::Separator();

	bool open = ImGui::TreeNodeEx(name.c_str(), flags);
	ImGui::PopStyleVar();

	ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
	if (ImGui::Button("+", { lineHeight, lineHeight })) {
		ImGui::OpenPopup("ComponentSettings");
	}

	bool removeRequested = false;
	if (ImGui::BeginPopup("ComponentSettings")) {
		if (ImGui::MenuItem("Remove Component")) {
			removeRequested = true;
		}
		ImGui::EndPopup();
	}

	if (open) {
		ui(component);
		ImGui::TreePop();
	}

	if (removeRequested) {
		entity->RemoveComponent<T>();
	}

	ImGui::PopID();
}

static void DrawBehaviour(const std::string& name, Entity* entity) {
	auto behaviour = entity->GetBehaviour(name);
	auto script = behaviour->GetScript();
	if (!behaviour || !script || !script->get().IsValid()) {
		ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "No valid script");
		return;
	}

	std::string label = script ? behaviour->path + ".lua" : "NONE";
	if (ImGui::Button(label.c_str()));

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SCRIPT")) {
			std::string droppedPath = (const char*)payload->Data;
		}
		ImGui::EndDragDropTarget();
	}

	sol::table table = *static_cast<Scripting::Script&>(*script).GetTable();

	ImGui::PushID(behaviour);

	bool changed = false;
	std::vector<ExposedVariable> exposedVars;

	for (const auto& [key, value] : table) {
		if (key.get_type() != sol::type::string) continue;
		std::string varName = key.as<std::string>();

		ExposedVarType varType = ExposedVarType::Unsupported;
		ExposedVariable var(varName, varType);

		if (value.is<bool>()) {
			var.type = ExposedVarType::Bool;
			var.value = value.as<bool>();
		}
		else if (value.is<int>()) {
			var.type = ExposedVarType::Int;
			var.value = value.as<int>();
		}
		else if (value.is<float>()) {
			var.type = ExposedVarType::Float;
			var.value = value.as<float>();
		}
		else if (value.is<Vector3>()) {
			var.type = ExposedVarType::Vector3;
			var.value = value.as<Vector3>();
		}

		if (var.type != ExposedVarType::Unsupported)
			exposedVars.push_back(var);
	}

	for (auto& var : exposedVars) {
		ImGui::PushID(var.name.c_str());

		switch (var.type) {
		case ExposedVarType::Bool:
			ImGui::Checkbox(var.name.c_str(), &std::get<bool>(var.value)); changed = true; break;
		case ExposedVarType::Int:
			ImGui::InputInt(var.name.c_str(), &std::get<int>(var.value)); changed = true; break;
		case ExposedVarType::Float:
			ImGui::InputFloat(var.name.c_str(), &std::get<float>(var.value), 0.1f); changed = true; break;
		case ExposedVarType::Vector3:
			Vector3& v = std::get<Vector3>(var.value);
			float data[3] = { v.x, v.y, v.z };
			if (ImGui::InputFloat3(var.name.c_str(), data)) {
				data[0] = v.x, data[1] = v.y; data[2] = v.z; changed = true;
			}
			break;
		}

		ImGui::PopID();
	}
	ImGui::PopID();
}

void SceneHierarchyPanel::DrawAllComponents(Entity* entity) {
	DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		const char* currentProjectionTypeString = /*projectionTypeStrings[(int)camera->GetProjectionType()]*/"Perspective";

		//if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
		//
		//	for (int i = 0; i < 2; i++) {
		//		bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
		//		if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
		//			currentProjectionTypeString = projectionTypeStrings[i];
		//			//camera->SetProjectionType((Camera::ProjectionType)i);
		//		}
		//
		//		if (isSelected)
		//			ImGui::SetItemDefaultFocus();
		//	}
		//
		//	ImGui::EndCombo();
		//}

		//if (camera->GetProjectionType() == Camera::ProjectionType::Perspective) {
			float fov = component->GetFov();
			if (ImGui::DragFloat("Field Of View", &fov)) {
				component->SetFov(fov);
				component->MarkAsDirty();
			}

			float nearPlane = component->GetNear();
			if (ImGui::DragFloat("Near Plane", &nearPlane)) {
				component->SetNear(nearPlane);
				component->MarkAsDirty();
			}

			float farPlane = component->GetFar();
			if (ImGui::DragFloat("Far Plane", &farPlane)) {
				component->SetFar(farPlane);
				component->MarkAsDirty();
			}

			//camera.SetProjectionMatrix(glm::perspective(fov, 1280.0f / 720.0f, nearPlane, farPlane));
		//}
		//if (camera->GetProjectionType() == Camera::ProjectionType::Orthographic) {
		//	
		//}
	});

	DrawComponent<MaterialRendererComp>("Material Renderer", entity, [&](auto& component) {
		MeshRendererComponent* modelRenderer = component->owner.GetComponent<MeshRendererComponent>();
		if (!modelRenderer || !modelRenderer->GetModel()) {
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No model assigned to entity");
			return;
		}

		const auto& meshIndices = modelRenderer->GetModel()->GetMeshIndices();
		const auto& materialNames = modelRenderer->GetModel()->GetMaterialNames();

		ImGui::Text("Assigned Materials:");
		ImGui::Separator();
		ImGui::Spacing();

		for (size_t listIdx = 0; listIdx < meshIndices.size(); listIdx++) {
			uint32_t meshIdx = meshIndices[listIdx];
			Mesh* mesh = AssetManagment::MeshManager::GetMeshByIndex(meshIdx);
			if (!mesh) continue;

			uint32_t matIndex = mesh->materialIndex;

			ImGui::PushID(static_cast<int>(matIndex));

			// Header with mesh name and material index
			ImGui::Text("%s", mesh->name.c_str());
			ImGui::SameLine();
			ImGui::TextDisabled("(Material Index: %u)", matIndex);

			// Material name and current assignment
			Rendering::Material* currentMat = component->GetMaterialByIndex(matIndex);
			std::string currentPath = currentMat ? currentMat->path : "";
			std::string displayName = (matIndex < materialNames.size()) ? materialNames[matIndex] : "Undefined";
			
			if (!currentPath.empty()) {
				displayName += " - " + std::filesystem::path(currentPath).stem().string();
			}
			else {
				displayName += " (Default/No material)";
			}
			ImGui::TextDisabled("%s", displayName.c_str());

			std::string buttonLabel = currentPath.empty() ? "Assign Material" : "Change Material";
			ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x * 0.65f, 0 };

			if (ImGui::Button(buttonLabel.c_str(), buttonSize)) {
				std::string filepath = FileDialogs::OpenFile("Material Files\0*.ftmat\0All Files\0*.*\0");
				if (!filepath.empty() && String::EndsWith(filepath, ".ftmat")) {
					if (Rendering::Material* newMat = Loaders::MaterialLoader::Create(filepath, true)) {
						component->RemoveMaterialByIndex(matIndex);
						component->AddMaterialByIndex(matIndex, *newMat);
					}
				}
			}

			// Drag & Drop
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MATERIAL")) {
					std::string droppedPath = static_cast<const char*>(payload->Data);
					if (Rendering::Material* newMat = Loaders::MaterialLoader::Create(droppedPath, true)) {
						component->RemoveMaterialByIndex(matIndex);
						component->AddMaterialByIndex(matIndex, *newMat);
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
				ImGui::OpenPopup("MatContextMenu");
			}

			if (ImGui::BeginPopup("MatContextMenu")) {
				if (ImGui::MenuItem("Set to Default")) {
					if (Rendering::Material* def = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true)) {
						component->RemoveMaterialByIndex(matIndex);
						component->AddMaterialByIndex(matIndex, *def);
					}
				}
				if (currentMat && ImGui::MenuItem("Edit Material")) {
					auto& editor = m_panelsManager.GetPanelAs<Moon::Editor::MaterialEditor>("Material Editor");
					editor.Open();
					editor.Focus();
					editor.SetMaterial(*currentMat, true);
				}
				if (currentMat && ImGui::MenuItem("Clear (remove override)")) {
					component->RemoveMaterialByIndex(matIndex);
				}
				ImGui::EndPopup();
			}

			ImGui::Spacing();
			ImGui::PopID();
		}
		});

	DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&entity](auto& component) {
		std::string modelName = "Undefined";
		std::string modelPath = "No model assigned";

		if (component->GetModel()) {
			modelName = std::filesystem::path(component->GetModel()->path).stem().string();
			modelPath = component->GetModel()->path;
		}

		// Disaplay name, path
		ImGui::Text("Model: %s", modelName.c_str());
		ImGui::TextDisabled("Path: %s", modelPath.c_str());

		if (component->GetModel()) {
			auto* model = component->GetModel();
			ImGui::TextDisabled("Meshes: %zu | Materials: %zu",
				model->GetMeshIndices().size(),
				model->GetMaterialNames().size());
		}

		ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, 0);
		std::string buttonLabel = component->GetModel() ? modelName : "Assign Model";
		if (ImGui::Button(buttonLabel.c_str(), buttonSize)) {
			std::string filepath = FileDialogs::OpenFile(
				"3D Models\0*.obj;*.fbx;*.gltf;*.glb\0All Files\0*.*\0"
			);
			if (!filepath.empty()) {
				component->ModelPath = filepath;
				auto newModel = Loaders::ModelLoader::Create(filepath, false);
				if (newModel) {
					component->SetModel(newModel);
					MaterialRendererComp* materialComp = component->owner.GetComponent<MaterialRendererComp>();
					if (materialComp == nullptr)
						materialComp = &component->owner.AddComponent<MaterialRendererComp>();
					Rendering::Material* material = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
					for (auto mesh : newModel->GetMeshIndices()) {
						materialComp->AddMaterialByIndex(AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex, *material);
					}
				}
			}
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MODEL")) {
				std::string droppedPath = static_cast<const char*>(payload->Data);
				component->ModelPath = droppedPath;
				auto newModel = Loaders::ModelLoader::Create(droppedPath, false);
				if (newModel) {
					component->SetModel(newModel);
					MaterialRendererComp* materialComp = component->owner.GetComponent<MaterialRendererComp>();
					if (materialComp == nullptr)
						materialComp = &component->owner.AddComponent<MaterialRendererComp>();
					Rendering::Material* material = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
					for (auto mesh : newModel->GetMeshIndices()) {
						materialComp->AddMaterialByIndex(AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex, *material);
					}
				}
			}
			else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SKINMODEL")) {
				std::string droppedPath = (const char*)payload->Data;
				auto newModel = Loaders::ModelLoader::Create(droppedPath, false);
				if (newModel) {
					component->SetModel(newModel);
					MaterialRendererComp* materialComp = component->owner.GetComponent<MaterialRendererComp>();
					if (materialComp == nullptr)
						materialComp = &component->owner.AddComponent<MaterialRendererComp>();
					Rendering::Material* material = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
					for (auto mesh : newModel->GetMeshIndices()) {
						materialComp->AddMaterialByIndex(AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex, *material);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	});

	DrawComponent<SkinnedMeshRendererComp>("Skinned Mesh Renderer", entity, [&entity](auto& component) {
		
		std::string modelName = component->GetModel() ? component->GetModel()->m_modelData.name : "Undefined";

		ImGui::Button(modelName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MODEL")) {
				std::string droppedPath = (const char*)payload->Data;

				//component->ModelPath = droppedPath;
				auto newModel = Loaders::SkinnedModelLoader::Create(droppedPath, false);
				if (newModel) {
					component->SetModel(newModel);
					MaterialRendererComp* materialComp = component->owner.GetComponent<MaterialRendererComp>();
					if (materialComp == nullptr)
						materialComp = &component->owner.AddComponent<MaterialRendererComp>();
					//Rendering::Material* material = Loaders::MaterialLoader::Create("data/engine/materials/default.ftmat", true);
					//for (auto mesh : newModel->GetMeshes()) {
					//	materialComp->AddMaterialByIndex(mesh->GetMaterialIndex(), *material);
					//}
				}
			}
			ImGui::EndDragDropTarget();
		}
	});

	DrawComponent<RigidBodyComponent>("RigidBody", entity, [&entity](auto& component) {
	
		// Mass/Density Control
		{
			//if (component->GetBodyType() == Moon::Physics::BodyType::Dynamic) {
				ImGui::Text("Mass");
				ImGui::SameLine();
				float currentMass = component->GetMass();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
				if (ImGui::DragFloat("##Mass", &currentMass, 0.1f, 0.0f, 1000.0f, "%.2f")) {
					component->SetMass(currentMass);
				}
				ImGui::SameLine();
				// Mass visualization
				if (currentMass <= 0.0f)
					ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Static (infinite mass)");
				else if (currentMass < 1.0f)
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "Light object");
				else if (currentMass < 5.0f)
					ImGui::TextColored(ImVec4(1, 1, 0, 1), "Medium object");
				else
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Heavy object");
			//}
		}

		ImGui::Spacing();

		// Flags Control
		{
			bool kinematic = component->IsKinematic();
			// Kinematic toggle
			ImGui::Text("Kinematic");
			ImGui::SameLine();
			if (ImGui::Checkbox("##IsKinematic", &kinematic)) {
				component->SetKinematic(kinematic);
			}
			// Trigger toggle
			ImGui::Text("Trigger");
			ImGui::SameLine();
			bool trigger = component->IsTrigger();
			if (ImGui::Checkbox("##IsTrigger", &trigger)) {
				component->SetTrigger(trigger);
			}
			// Status display based on flags
			ImGui::Spacing();
			ImGui::TextDisabled("Status: ");
			ImGui::SameLine();
			if (kinematic && trigger)
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Kinematic Trigger");
			else if (kinematic) 
				ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "Kinematic");
			else if (trigger) 
				ImGui::TextColored(ImVec4(0.5f, 0, 1, 1), "Trigger");
			else 
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Dynamic");
		}

		// Physics Material
		if (ImGui::TreeNodeEx("Physics Material")) {
			float bounciness = component->GetBounciness();
			if (ImGui::DragFloat("Bounciness", &bounciness, 0.01f, 0.0f, 1.0f, "%.2f")) {
				component->SetBounciness(bounciness);
			}

			// Combined Friction slider
			float friction = component->GetFriction();
			if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 1.0f, "%.2f")) {
				component->SetFriction(friction);
			}

			// Separate Static/Dynamic friction
			ImGui::Indent();
			float staticFriction = component->GetStaticFriction();
			if (ImGui::DragFloat("Static Friction", &staticFriction, 0.01f, 0.0f, 1.0f, "%.2f")) {
				component->SetStaticFriction(staticFriction);
			}

			float dynamicFriction = component->GetDynamicFriction();
			if (ImGui::DragFloat("Dynamic Friction", &staticFriction, 0.01f, 0.0f, 1.0f, "%.2f")) {
				component->SetDynamicFriction(dynamicFriction);
			}
			ImGui::Unindent();

			ImGui::TreePop();
		}

		// Motion Constraints
		if (ImGui::TreeNodeEx("Motion Constraints", ImGuiTreeNodeFlags_DefaultOpen)) {

			// Placeholder for contraint controls
			// These would control freeze positions/rotations
			// Get current constraints
			glm::bvec3 freezePos = component->GetFreezePosition();
			glm::bvec3 freezeRot = component->GetFreezeRotation();

			bool posChanged = false;
			ImGui::Text("Freeze Position:");
			ImGui::SameLine();
			ImGui::BeginGroup();
			if (ImGui::Checkbox("X##FreezePosX", &freezePos.x)) posChanged = true;
			ImGui::SameLine();
			if (ImGui::Checkbox("Y##FreezePosY", &freezePos.y)) posChanged = true;
			ImGui::SameLine();
			if (ImGui::Checkbox("Z##FreezePosZ", &freezePos.z)) posChanged = true;
			ImGui::EndGroup();

			ImGui::Spacing();

			bool rotChanged = false;
			ImGui::Text("Freeze Rotation:");
			ImGui::SameLine();
			ImGui::BeginGroup();
			if (ImGui::Checkbox("X##FreezeRotX", &freezeRot.x)) rotChanged = true;
			ImGui::SameLine();
			if (ImGui::Checkbox("Y##FreezeRotY", &freezeRot.y)) rotChanged = true;
			ImGui::SameLine();
			if (ImGui::Checkbox("Z##FreezeRotZ", &freezeRot.z)) rotChanged = true;
			ImGui::EndGroup();

			// Apply changes
			if (posChanged) component->SetFreezePosition(freezePos);
			if (rotChanged) component->SetFreezeRotation(freezeRot);

			// Status display
			ImGui::Spacing();
			ImGui::Text("Constraint Status:");
			ImGui::SameLine();

			std::string constraintStatus;
			if (freezePos.x || freezePos.y || freezePos.z || freezeRot.x || freezeRot.y || freezeRot.z) {
				constraintStatus = "Partially Constrained";
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s", constraintStatus.c_str());
			}
			else {
				constraintStatus = "Fully Free";
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", constraintStatus.c_str());
			}

			ImGui::TreePop();
		}

		// Advanced Properties
		if (ImGui::TreeNodeEx("Advanced Properties", ImGuiTreeNodeFlags_DefaultOpen)) {

			// Drag (linear damping)
			float linearDrag = component->GetLinearDrag();
			if (ImGui::DragFloat("Linear Drag", &linearDrag, 0.01f, 0.0f, 1.0f, "%.2f"))
				component->SetLinearDrag(linearDrag);

			// Angular drag
			float angularDrag = component->GetAngularDrag();
			if (ImGui::DragFloat("Angular Drag", &angularDrag, 0.01f, 0.0f, 1.0f, "%.2f"))
				component->SetAngularDrag(angularDrag);

			// Gravity scale
			float gravityScale = component->GetGravityScale();
			if (ImGui::DragFloat("Gravity Scale", &gravityScale, 0.1f, 0.0f, 5.0f, "%.1f"))
				component->SetGravityScale(gravityScale);

			ImGui::Text("Drag Status: ");
			ImGui::SameLine();
			// Drag status
			std::string dragStatus;
			if (linearDrag < 0.05f && angularDrag < 0.05f) {
				dragStatus = "Low Drag";
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", dragStatus.c_str());
			}
			else if (linearDrag < 0.2f && angularDrag < 0.2f) {
				dragStatus = "Medium Drag";
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", dragStatus.c_str());
			}
			else {
				dragStatus = "High Drag";
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s", dragStatus.c_str());
			}

			ImGui::Text("Gravity Status: ");
			ImGui::SameLine();

			// Gravity status
			std::string gravityStatus;
			if (gravityScale == 0.0f) {
				gravityStatus = "No Gravity";
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 1, 1), "%s", gravityStatus.c_str());
			}
			else if (gravityScale == 1.0f) {
				gravityStatus = "Normal Gravity";
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", gravityStatus.c_str());
			}
			else if (gravityScale > 1.0f) {
				gravityStatus = "High Gravity (" + std::to_string(gravityScale) + "x)";
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s", gravityStatus.c_str());
			}
			else {
				gravityStatus = "Low Gravity (" + std::to_string(gravityScale) + "x)";
				ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "%s", gravityStatus.c_str());
			}

			ImGui::TreePop();
		}

		// Sleep Settings
		if (ImGui::TreeNodeEx("Sleep Settings")) {

			bool enableSleep = component->GetEnableSleep();
			if (ImGui::Checkbox("Enable Sleep", &enableSleep))
				component->SetEnableSleep(enableSleep);

			if (enableSleep) {
				float sleepThreshold = component->GetSleepThreshold();
				if (ImGui::DragFloat("Sleep Threshold", &sleepThreshold, 0.01f, 0.0f, 1.0f, "%.2f")) {
					component->SetSleepThreshold(sleepThreshold);
				}
			}

			if (component->IsSleeping()) {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 1, 1), "Sleeping");
				if (ImGui::Button("Wake Up")) {
					component->WakeUp();
				}
			}
			else {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Awake");
				if (enableSleep && ImGui::Button("Put To Sleep")) {
					component->PutToSleep();
				}
			}

			ImGui::TreePop();
		}
	});

	DrawComponent<BoxColliderComponent>("Box Collider", entity, [&entity](auto& component) {

		glm::vec3 halfExtents = component->halfExtents;
		DrawVec3Control("Size", &halfExtents);
		component->SetSize(halfExtents);

		glm::vec3 shapeOffset = component->shapeOffset;
		DrawVec3Control("ShapeOffset", &shapeOffset);
		if (shapeOffset != component->shapeOffset) {
			component->SetShapeOffset(shapeOffset);
		}
	});

	DrawComponent<CharacterControllerComponent>("Character Controller", entity, [&entity](auto& component) {
		auto& settings = component->GetCharacterController()->GetSettings();

		ImGui::Text("Shape Limit:");
		ImGui::SameLine();
		if (ImGui::DragFloat("##CShapeLimit", &settings.stepOffset, 0.01f, 0.0f, 20.0f)) {
			component->GetCharacterController()->Recreate();
		}

		ImGui::Text("Radius:");
		ImGui::SameLine();
		if (ImGui::DragFloat("##CRadius", &settings.radius, 0.01f, 0.001f, 10.0f)) {
			component->GetCharacterController()->Recreate();
		}

		ImGui::Text("Height:");
		ImGui::SameLine();
		if (ImGui::DragFloat("##CHeight", &settings.height, 0.01f, 0.3f, 20.0f)) {
			component->GetCharacterController()->Recreate();
		}

		ImGui::Text("ScaleCoeff:");
		ImGui::SameLine();
		if (ImGui::DragFloat("##CScaleCoeff", &settings.scaleCoeff, 0.01f, 0.3f, 20.0f)) {
			component->GetCharacterController()->Recreate();
		}
	});

	DrawComponent<PointLightComponent>("Point Light", entity, [&entity](auto& component) {
		
		glm::vec3 color = component->GetColor();
		if (ImGui::ColorEdit3("Color", &color[0]))
			component->SetColor(color);

		ImGui::Text("Radius:");
		ImGui::SameLine();
		ImGui::DragFloat("##Radius", &component->GetData().radius, 0.1f, 0.0f, 100.0f);

		float strength = component->GetData().strength;
		ImGui::Text("Strength:");
		ImGui::SameLine();
		if (ImGui::DragFloat("##Strength", &strength, 0.05f, 0.0f, 100.0f, "%.2f"))
			component->SetStrength(strength);
		});

	DrawComponent<DirectionalLightComp>("Directional Light", entity, [&entity](auto& component) {
		auto& light = component->GetData();

		// Color & Intensity
		glm::vec3 color = light.color;
		if (ImGui::ColorEdit3("Color", &color[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float))
			component->SetColor(color);

		float strength = light.strength;
		if (ImGui::DragFloat("Intensity", &strength, 0.1f, 0.0f, 100.0f, "%.2f lux"))
			component->SetStrength(strength);

		ImGui::Separator();

		// Direction Visualization (Read-Only)
		glm::vec3 dir = light.direction;
		ImGui::Text("Direction:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "(%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
		

		});

	DrawComponent<AudioComponent>("Audio Source", entity, [&entity](auto& component) {

		std::string label = component->filepath.empty() ? "Empty" : component->filepath;

		if (ImGui::Button(label.c_str())) {
			component->Play();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_AUDIO")) {
				std::string droppedPath = (const char*)payload->Data;
				component->filepath = FileSystem::AbsoluteToRelative(droppedPath);
			}
			ImGui::EndDragDropTarget();
		}

		auto autoPlay = component->IsAutoplayed();
		if (ImGui::Checkbox("AutoPlay", &autoPlay)) {
			component->SetAutoplay(autoPlay);
		}

		auto spatial = component->IsSpatial();
		if (ImGui::Checkbox("Spatial", &spatial)) {
			component->SetSpatial(autoPlay);
		}
	});

	DrawComponent<AudioListenerComponent>("Audio Listener", entity, [&entity](auto& component) {
	});

	DrawComponent<Text2DComponent>("Text 2D", entity, [&entity](auto& component) {
		// Text input
		std::string text = component->GetText();
		if (ImGui::InputTextMultiline("##Text", &text,
			ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3))) {
			component->SetText(text);
		}

		// Color picker
		glm::vec3 color = component->GetColor();
		if (ImGui::ColorEdit3("Color", &color[0])) {
			component->SetColor(color);
		}

		// Font properties
		static char fontBuffer[256] = {};
		if (fontBuffer[0] == '\0') {
			strncpy(fontBuffer, component->GetFontPath().c_str(), 255);
		}

		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("##FontPath", fontBuffer, sizeof(fontBuffer))) {
			component->SetFont(fontBuffer, component->GetFontSize());
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_FONT")) {
				const char* path = (const char*)payload->Data;
				strncpy(fontBuffer, path, 255);
				component->SetFont(path, component->GetFontSize());
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::PopItemWidth();

		float fontSize = component->GetFontSize();
		if (ImGui::DragFloat("Font Size", &fontSize, 0.5f, 1.0f, 72.0f)) {
			component->SetFont(component->GetFontPath(), fontSize);
		}

		// Info
		if (component->GetFont()) {
			ImGui::TextDisabled("Font loaded: %s (%.0fpt)",
				component->GetFontPath().c_str(), component->GetFontSize());
		}
		else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Font failed to load!");
		}
	});

	DrawComponent<UI::WidgetComponent>("Widget", entity, [&entity](auto& component) {
		ImGui::Text("Texture:");
		if (component->GetTexture()) {
			ImGui::Image((void*)(intptr_t)component->GetTexture()->GetTexture().GetID(), ImVec2(64, 64));
		}
		else {
			ImGui::TextDisabled("No texture assigned");
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TEXTURE")) {
				std::string path = static_cast<const char*>(payload->Data);
				auto texture = Loaders::TextureLoader::Create(path);
				if (texture) {
					component->SetTexture(texture);
				}
			}
			ImGui::EndDragDropTarget();
		}

		glm::vec3 color = component->GetColor();
		if (ImGui::ColorEdit3("Color", &color[0])) {
			component->SetColor(color);
		}
		});
}

void SceneHierarchyPanel::DrawAllBehaviours(Moon::Entity* entity) {
	std::string behaviourToRemove;

	for (const auto& [name, behaviour] : entity->GetBehaviours()) {
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_AllowItemOverlap |
			ImGuiTreeNodeFlags_FramePadding;

		ImGui::PushID(name.c_str());

		ImVec2 contentRegion = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();

		bool open = ImGui::TreeNodeEx(name.c_str(), flags);
		ImGui::PopStyleVar();

		ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
		if (ImGui::Button("+", { lineHeight, lineHeight })) {
			ImGui::OpenPopup("ComponentSettings");
		}

		if (ImGui::BeginPopup("ComponentSettings")) {
			if (ImGui::MenuItem("Remove Behaviour")) {
				behaviourToRemove = name;
			}
			ImGui::EndPopup();
		}
		if (open) {
			DrawBehaviour(name, entity);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (!behaviourToRemove.empty())
		entity->RemoveBehaviour(behaviourToRemove);
}

template<typename T>
void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
{
	if (!_selectionContext->GetComponent<T>()) {
		if (ImGui::MenuItem(entryName.c_str())) {
			_selectionContext->AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}