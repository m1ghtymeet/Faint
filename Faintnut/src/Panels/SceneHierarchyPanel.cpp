#include "SceneHierarchyPanel.h"
#include "Core/Object/Object.h"
#include "Core/String.h"
#include "Core/OS.h"
#include "Util/PlatformUtil.h"
#include "UI/ImUI.h"
#include "../Helpers/PopupHelper.h"
#include "Engine.h"

#include "../Helpers/ImGuiHelper.h"

// Resources
#include "AssetManagment/AssetManager.h"
#include "AssetManagment/FontAwesome5.h"

#include <Faint.h>
#include <imgui/imgui_stdlib.h>
#include <filesystem>

// Components
#include "Scene/Components.h"
#include "Scene/Components/PrefabComponent.h"
#include "Scene/Components/RigidBodyComponent.h"

// Components Panel
#include "../ComponentsPanel/ModelResourceInspector.h"
#include "../ComponentsPanel/NetScriptPanel.h"
#include "../ComponentsPanel/ComponentPanel.h"
#include "../ComponentsPanel/WrenScriptPanel.h"

#ifdef _MSVC_LANG
	#define _CRT_SECURE_NO_WARNINGS
#endif

extern const std::filesystem::path s_assetPath;
bool EntityChanged = false;

Scope<ModelResourceInspector> _modelInspector;
std::string _importedModelPath;
bool _expanded = false;

Scope<NetScriptPanel> netPanel;
Scope<WrenScriptPanel> scriptPanel;

SceneHierarchyPanel::SceneHierarchyPanel(const std::string& p_title, bool p_opened) {
	
}

//void SceneHierarchyPanel::SelectEntityByInstance(Entity& p_gb) {
//	_selectionContext = &p_gb;
//}

void SceneHierarchyPanel::Draw() {

	if (ImGui::Begin("Scene Hierarchy")) {		
		for (Entity* entity : Engine::GetCurrentScene()->GetAllEntities()) {
			if (!entity->HasParent()) {
				/* Draw Entity Node */
				{
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

					ImGuiTreeNodeFlags flags = ((_selectionContext != nullptr && _selectionContext->GetID() == entity->GetID()) ? ImGuiTreeNodeFlags_Selected : 0)
						| ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

					ImGui::TableNextColumn();

					if (entity->GetChildren().size() <= 0) {
						flags |= ImGuiTreeNodeFlags_Leaf;
					}

					bool hasChildren = !entity->GetChildren().empty();
					bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity->GetID(), flags, entity->GetName().c_str());
					
					if (ImGui::IsItemClicked())
						SetSelectedEntity(entity);

					ImGui::TreePop();

					ImGui::PopStyleVar();
				}
			}
		}

		// Right click on empty space for adding new entity
		if (ImGui::IsMouseDown(1) && ImGui::IsWindowHovered()) {
			ImGui::OpenPopup("AddEntity");
		}
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 3));
		if (ImGui::BeginPopup("AddEntity")) {
			if (ImGui::BeginMenu("Create...")) {
				if (ImGui::MenuItem("Empty Entity")) {
					Entity& newEntity = Engine::GetCurrentScene()->CreateEntity("Empty Entity");
					SetSelectedEntity(&newEntity);
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		
		}
		ImGui::PopStyleVar(3);
	}
	ImGui::End();

	ImGui::Begin("Properties");
	if (_selectionContext) {
		DrawComponents(_selectionContext);
	}
	ImGui::End();
}
Entity* SceneHierarchyPanel::GetSelectedEntity() const {
	return _selectionContext;
}

void SceneHierarchyPanel::SetSelectedEntity(Entity* entity) {
	_selectionContext = entity;
	EntityChanged = true;
}

void SceneHierarchyPanel::SetStatusMessage(const std::string& message, Color color) {
	m_StatusMessage = message;
	m_StatusBarColor = color;
}

template<typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity* entity, UIFunction ui) {
	
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity->GetComponent<T>()) {
		ImGui::PushID(name.c_str());

		auto* component = entity->GetComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings")) {
			if (ImGui::MenuItem("Remove Component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open) {
			ui(component);
			ImGui::TreePop();
		}
		//if (name != "Transform")
		//	if (removeComponent)
		//		entity->RemoveComponent<T>();

		ImGui::PopID();
	}
}

void SceneHierarchyPanel::DrawComponents(Entity* entity) {

	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	strcpy_s(buffer, sizeof(buffer), entity->GetName().c_str());
	if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
		entity->SetName(std::string(buffer));
	}
	

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);
	

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponent");

	if (ImGui::BeginPopup("AddComponent")) {
		DisplayAddComponentEntry<CameraComponent>("Camera");
		DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
		DisplayAddComponentEntry<SkinnedMeshRendererComponent>("Skinned Mesh Renderer");
		DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
		DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
		DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller");
		DisplayAddComponentEntry<RigidBodyComponent>("RigidBody");
		DisplayAddComponentEntry<NetScriptComponent>("Net Script");
		//DisplayAddComponentEntry<WrenScriptComponent>("Wren Script");
		//DisplayAddComponentEntry<LuaScriptComponent>("Lua Script");
		DisplayAddComponentEntry<AudioComponent>("Audio");
		DisplayAddComponentEntry<LightComponent>("Light");
		//DisplayAddComponentEntry<SkyComponent>("Sky");

		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();

	DrawComponent<TransformComponent>("Transform", entity, [&entity](auto& component) {
		
		glm::vec3 position = component->GetTransform().m_localPosition;
		DrawVec3Control("Position", &position);
		component->SetLocalPosition(position);
	
		Quat currentRotation = component->GetLocalRotation();
		Vec3 eulerDegreesOld = glm::degrees(glm::eulerAngles(currentRotation));
		DrawVec3Control("Rotation", &eulerDegreesOld);
		Vec3 eulerDelta = eulerDegreesOld - glm::degrees(glm::eulerAngles(currentRotation));
	
		// Apply a small threshold to ignore minor changes
		if (fabs(eulerDelta.x) < 0.01) eulerDelta.x = 0.0f;
		if (fabs(eulerDelta.y) < 0.01) eulerDelta.y = 0.0f;
		if (fabs(eulerDelta.z) < 0.01) eulerDelta.z = 0.0f;
		if (glm::length(eulerDelta) > 0.001f)
		{
			// Convert delta back to radians
			Vec3 eulerAnglesDelta = glm::radians(eulerDelta);
	
			// Calculate the new rotation quaternion by applying the delta to the current rotation
			// Apply changes only to the axis that was modified
			Quat deltaRotation = glm::quat(eulerAnglesDelta);
			Quat deltaRotationX = glm::angleAxis(eulerAnglesDelta.x, Vec3(1.0f, 0.0f, 0.0f));
			Quat deltaRotationY = glm::angleAxis(eulerAnglesDelta.y, Vec3(0.0f, 1.0f, 0.0f));
			Quat deltaRotationZ = glm::angleAxis(eulerAnglesDelta.z, Vec3(0.0f, 0.0f, 1.0f));
	
			// Combine the rotations: only the modified axis is affected
			Quat newRotation = deltaRotationZ * deltaRotationY * deltaRotationX * currentRotation;
	
			// Set the new rotation
			component->SetLocalRotation(newRotation);
		}
	
		glm::vec3 scale = component->GetTransform().m_localScale;
		DrawVec3Control("Scale", &scale);
		component->SetLocalScale(scale);
	});

	DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
		auto& camera = component->camera;

		const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		const char* currentProjectionTypeString = /*projectionTypeStrings[(int)camera->GetProjectionType()]*/"Perspective";

		if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {

			for (int i = 0; i < 2; i++) {
				bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
				if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
					currentProjectionTypeString = projectionTypeStrings[i];
					//camera->SetProjectionType((Camera::ProjectionType)i);
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		//if (camera->GetProjectionType() == Camera::ProjectionType::Perspective) {
			float fov = camera->m_fieldOfView;
			if (ImGui::DragFloat("Field Of View", &fov))
				camera->m_fieldOfView = fov;

			float nearPlane = camera->m_nearPlane;
			if (ImGui::DragFloat("Near Plane", &nearPlane))
				camera->m_nearPlane = nearPlane;

			float farPlane = camera->m_farPlane;
			if (ImGui::DragFloat("Far Plane", &farPlane))
				camera->m_farPlane = farPlane;

			camera->SetProjectionMatrix(glm::perspective(fov, Window::Get()->viewportWidth / Window::Get()->viewportHeight, nearPlane, farPlane));
		//}
		//if (camera->GetProjectionType() == Camera::ProjectionType::Orthographic) {
		//	
		//}
	});

	DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&entity](auto& component) {
		std::string label = "None";

		const bool isModelNone = component->GetModel() == nullptr;
		if (!isModelNone) {
			label = component->GetModel()->Name;
		}

		if (EntityChanged) {
			//_modelInspector = CreateScope<ModelResourceInspector>(component->GetModel());
			EntityChanged = false;
		}

		//if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			if (!isModelNone) {
		//		if (!_expanded) {
					//_modelInspector = CreateScope<ModelResourceInspector>(component->GetModel());
		//		}
		//		_expanded = !_expanded;
			}
		//}

		//if (_expanded) {
			//_modelInspector->Draw();
		//}

		bool shouldConvert = false;
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model")) {
				char* file = (char*)payload->Data;
				std::string fullPath = std::string(file, 256);

				component->ModelPath = fullPath;
				

				_importedModelPath = fullPath;

				Model* modelResource = AssetManager::LoadModel(fullPath, true);
				shouldConvert = true;
			}
			ImGui::EndDragDropTarget();
		}

		if (PopupHelper::ConfirmationDialog("##ConvertAsset", "Convert Asset")) {
			// Convert to disk
			Model* modelResource = AssetManager::LoadModel(_importedModelPath, true);
			json serializeData = modelResource->SerializeData();

			//std::cout << String::Split(_importedModelPath, '.')[0] << "\n";
			const std::string exportedMeshPath = String::Split(_importedModelPath, '.')[0] + ".mesh";
			FileSystem::BeginWriteFile(exportedMeshPath, true);
			FileSystem::WriteLine(serializeData.dump());
			FileSystem::EndWriteFile();
			
			component->ModelPath = exportedMeshPath;
			//component->_Model = modelResource;
		}

		if (shouldConvert) {
			PopupHelper::OpenPopup("##ConvertAsset");
		}
	});

	DrawComponent<SkinnedMeshRendererComponent>("Skinned Mesh Renderer", entity, [&entity](auto& component) {
		//bool shouldConvert = false;
		//if (ImGui::BeginDragDropTarget()) {
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_SkinnedModel")) {
		//		char* file = (char*)payload->Data;
		//		std::string fullPath = std::string(file, 256);
		//
		//		component->ModelPath = fullPath;
		//
		//		_importedModelPath = fullPath;
		//
		//		Ref<SkinnedModel> modelResource = AssetManager::LoadSkinnedModel(fullPath, true);
		//		component->Model = modelResource;
		//		shouldConvert = true;
		//	}
		//	ImGui::EndDragDropTarget();
		//}
	});

	DrawComponent<BoxColliderComponent>("Box Collider", entity, [&entity](auto& component) {
		DrawVec3Control("Size", &component->halfExtents);

		ImGui::Columns(3);
		ImGui::SetColumnWidth(0, 100);
		ImGui::Text("Is Trigger");
		ImGui::NextColumn();
		component->IsTrigger = UI::CheckBox("Is Trigger", component->IsTrigger);
		ImGui::Columns(1);
	});

	DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [&entity](auto& component) {

		ImGui::DragFloat("Radius", &component->Radius, 0.1f);
	});

	DrawComponent<CharacterControllerComponent>("Character Controller", entity, [&entity](auto& component) {
	
		
	});

	DrawComponent<RigidBodyComponent>("RigidBody", entity, [&entity](auto& component) {
		{
			ImGui::Text("Mass");
			ImGui::SameLine();

			ImGui::DragFloat("##Mass", &component->mass, 0.01f, 0.1f);
			component->mass = std::max(component->mass, 0.0f);
		}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock X axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##lockx", &component->LockX);
		//}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock Y axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##locky", &component->LockY);
		//}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock Z axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##lockz", &component->LockZ);
		//}
	});

	DrawComponent<NetScriptComponent>("Net Script", entity, [&entity](auto& component) {

		//netPanel->Draw(entity);
	});

	DrawComponent<LuaScriptComponent>("Lua Script", entity, [&entity](auto& component) {

		//component->path = "";
		
	});

	DrawComponent<LightComponent>("Point Light", entity, [&entity](auto& component) {
		
		BeginComponentTable(LIGHT, LightComponent);
		{
			{
				ImGui::Text("Color");

				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##lightColor", &component->Color.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				ComponentTableReset(component->Color, Vec3(1, 1, 1));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Strength");

				ImGui::TableNextColumn();
				ImGui::DragFloat("##Strength", &component->Strength, 0.1f, 0.0f, 100.0f);

				ImGui::TableNextColumn();
				ComponentTableReset(component->Strength, 1.f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Type");

				ImGui::TableNextColumn();
				const char* lightTypes[] = { "Point", "Directional", "Spot" };
				ComponentDropDown(lightTypes, LightType, component->Type);

				ImGui::TableNextColumn();
				ComponentTableReset(component->Type, LightType::Point);
			}
			ImGui::TableNextColumn();

			if (component->Type == Faint::LightType::Point) {
				{
					ImGui::Text("Radius");

					ImGui::TableNextColumn();
					ImGui::DragFloat("##Radius", &component->Radius, 0.1f, 0.0f, 100.0f);

					ImGui::TableNextColumn();
					ComponentTableReset(component->Radius, 1.f);
				}
			}
			else if (component->Type == Faint::LightType::Directional)
			{
				{
					ImGui::Text("Sync Direction with sky");

					ImGui::TableNextColumn();
					ImGui::Checkbox("##SyncSky", &component->SyncDirectionWithSky);

					ImGui::TableNextColumn();
					ComponentTableReset(component->SyncDirectionWithSky, false);

					// Light direction is only useful if it is not overriden by the procedural sky direction
					if (!component->SyncDirectionWithSky)
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("Direction");

							ImGui::TableNextColumn();
							//ImGuiHelper::DrawVec3("Direction", &component->Direction);

							ImGui::TableNextColumn();
							ComponentTableReset(component->Direction, Vec3(0, -1, 0));
						}
					}
				}
			}
			else if (component->Type == Faint::LightType::Spot)
			{
				{
					ImGui::Text("Cutoff");
					ImGui::TableNextColumn();

					ImGui::DragFloat("##cutoff", &component->Cutoff, 1.0f, 0.0f, 360.0f);
					ImGui::TableNextColumn();

					// Clamp inner angle so it doesnt exceed outer angle.
					component->Cutoff = glm::min(component->Cutoff, component->Outercutoff);

					ComponentTableReset(component->Cutoff, 12.5f);
					ImGui::TableNextColumn();
				}

				{
					ImGui::Text("Outer Cutoff");
					ImGui::TableNextColumn();

					ImGui::DragFloat("##outercutoff", &component->Outercutoff, 1.0f, component->Cutoff, 360.0f);
					ImGui::TableNextColumn();

					ComponentTableReset(component->Outercutoff, 30.0f);
					ImGui::TableNextColumn();
				}
			}
		}
		EndComponentTable();
	});

	DrawComponent<AudioComponent>("Audio", entity, [&entity](auto& component) {

		BeginComponentTable(AUDIO, AudioComponent);
		{
			{
				ImGui::Text("Audio File");
				ImGui::TableNextColumn();

				std::string path = component->FilePath;
				ImGui::Button(path.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 });
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Audio")) {

						char* file = (char*)payload->Data;
						std::string fullPath = std::string(file, 256);
						path = FileSystem::AbsoluteToRelative(fullPath);
						component->FilePath = path;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::TableNextColumn();

				ComponentTableReset(component->FilePath, "");
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Playing");
				ImGui::TableNextColumn();

				UI::CheckBox("##Player", component->IsPlaying);
				ImGui::TableNextColumn();

				ComponentTableReset(component->IsPlaying, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Loop");
				ImGui::TableNextColumn();

				UI::CheckBox("##Loop", component->Loop);
				ImGui::TableNextColumn();

				ComponentTableReset(component->Loop, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Volume");
				ImGui::TableNextColumn();

				UI::FloatSlider("##Volume", component->Volume, 0.0f, 2.0f, 0.001f);
				ImGui::TableNextColumn();

				ComponentTableReset(component->Volume, 1.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Playback Speed");
				ImGui::TableNextColumn();

				UI::FloatSlider("##PlaybackSpeed", component->PlaybackSpeed, 0.0001f, 1.0f, 0.01f);
				ImGui::TableNextColumn();

				ComponentTableReset(component->PlaybackSpeed, 1.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Pan");
				ImGui::TableNextColumn();

				UI::FloatSlider("##Pan", component->Pan, -1.0f, 1.0f, 0.01f);
				ImGui::TableNextColumn();

				ComponentTableReset(component->Pan, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Spatialized");
				ImGui::TableNextColumn();

				UI::CheckBox("##Spatialized", component->Spatialized);
				ImGui::TableNextColumn();

				ComponentTableReset(component->Spatialized, false);
			}

			if (component->Spatialized) {
				ImGui::TableNextColumn();
				{
					ImGui::Text("Min Distance");
					ImGui::TableNextColumn();

					UI::FloatSlider("##minDistance", component->MinDistance, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component->MinDistance, 1.0f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Max Distance");
					ImGui::TableNextColumn();

					UI::FloatSlider("##maxDistance", component->MaxDistance, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component->MaxDistance, 10.0f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Attenuation Factor");
					ImGui::TableNextColumn();

					UI::FloatSlider("##attenuationFactor", component->AttenuationFactor, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component->AttenuationFactor, 1.0f);
				}
			}
		}
		EndComponentTable();
	});

	DrawComponent<WrenScriptComponent>("Wren Script", entity, [&entity](auto& component) {

		//scriptPanel->Draw(entity);
	});

	DrawComponent<SkyComponent>("Sky", entity, [&entity](auto& component) {

		
	});
}

/*void SceneHierarchyPanel::DrawEntityTree(Entity* entity, bool drawChildren) {

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
	
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

	std::string name = entity->GetName();

	//if (_selectionContext->GetID() == entity->GetID())
	//	base_flags |= ImGuiTreeNodeFlags_Selected;

	ImGui::TableNextColumn();

	// If has no children draw tree node leaf
	bool isPrefab = entity->GetComponent<PrefabComponent>();
	if (entity->GetChildren().size() <= 0 || !drawChildren) {
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	//if (namecomponent->isPrefab && entity.HasComponent<PrefabComponent>()) {
	//	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	//}
	//else if (entity.HasComponent<BSPBrushComponent>()) {
	//	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
	//}

	auto cursorPos = ImGui::GetCursorPos();
	ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
	const auto& cleanName = String::RemoveWhiteSpace(String::ToUpper(name));
	const size_t searchIt = cleanName.find(String::RemoveWhiteSpace(String::ToUpper(searchQuery)));

	ImGui::SetNextItemAllowOverlap();
	bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

	if (!searchQuery.empty() && searchIt != std::string::npos) {
		int firstLetterFoundIndex = static_cast<int>(searchIt);

		const auto foundStr = name.substr(0, firstLetterFoundIndex + searchQuery.size());

		auto fg = ImGui::GetForegroundDrawList();

		//fg->AddRectFilled(ImVec2(cursorPosition.x + 20.0f, cursorPosition.y + 4.0f), ImVec2());
	}

	if (m_IsRenaming) {
		if (_selectionContext->GetID() == entity->GetID()) {
			ImGui::SetCursorPosY(cursorPos.y);
			ImGui::Indent();
			ImGui::InputText("##renamingEntity", &name);
			ImGui::Unindent();
			if (Input::KeyPressed(Key::Enter)) {
				entity->SetName(name);
				m_IsRenaming = false;
			}
		}
	}

	bool isDragging = false;
	//if (namecomponent->isPrefab && entity.HasComponent<PrefabComponent>()) {
	//	ImGui::PopStyleColor();
	//}

	if (!m_IsRenaming && ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("ENTITY", (void*)&entity, sizeof(Entity));
		ImGui::Text(name.c_str());
		ImGui::EndDragDropSource();
	}

	if (!isPrefab && ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
			//Entity payload_entity = *(const Entity)payload->Data;

			// Check if entity is already parent.
			//if (!payload_entity.EntityContainsItself(payload_entity, entity) && payload_entity.GetParent()->GetID() != entity.GetID() && std::count(payload_entity.GetChildren().begin(), payload_entity.GetChildren().end(), payload_entity) == 0) {
			//	if (payload_entity.HasParent()) {
			//		// Erase remove idiom.
			//		Entity& childOfParent = payload_entity;
			//		childOfParent.GetChildren().erase(std::remove(childOfParent.GetChildren().begin(), childOfParent.GetChildren().end(), payload_entity), childOfParent.GetChildren().end());
			//	}
			//
			//	payload_entity.SetParent(entity);
			//	payload_entity.GetChildren().push_back(&payload_entity);
			//}
		}
		else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CSharp")) {
			char* file = (char*)payload->Data;

			std::string fullPath = std::string(file, 512);
			std::string path = FileSystem::AbsoluteToRelative(std::move(fullPath));

			if (entity->GetComponent<NetScriptComponent>()) {
				entity->GetComponent<NetScriptComponent>()->ScriptPath = path;
			} else {
				entity->AddComponent<NetScriptComponent>().ScriptPath = path;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (!isDragging && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		// We selected another another that ew weren't renaming
		if (_selectionContext->GetID() != entity->GetID()) {
			m_IsRenaming = false;
		}

		_selectionContext = entity;
		EntityChanged = true;
	}

	if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))) {
		m_IsRenaming = true;
	}

	if (ImGui::BeginPopupContextItem()) {
		_selectionContext = entity;

		if (_selectionContext->GetComponent<CameraComponent>()) {
			// Moves the editor camera to the camera position and direction.
			if (ImGui::Selectable("Focus camera")) {
				
			}
			ImGui::Separator();
		}

		if (ImGui::Selectable("Remove")) {
			//QueueDeletion = entity;
		}

		if (entity->HasParent() && ImGui::Selectable("Move to root")) {
			//auto& parentComp = _selectionContext.GetComponent<ParentComponent>();
			if (_selectionContext->HasParent()) {
				auto parentParentComp = _selectionContext->GetParent();
				//parentParentComp.RemoveChildren(entity);
				//parentComp.HasParent = false;
			}
		}

		if (!isPrefab && ImGui::Selectable("Save entity as a new prefab")) {
			//Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(_selectionContext);
			//std::string savePath = FileDialogs::SaveFile("*.prefab");
			//if (!String::EndsWith(savePath, ".prefab")) {
			//	savePath += ".prefab";
			//}
			//
			//if (!savePath.empty()) {
			//	newPrefab->SaveAs(FileSystem::AbsoluteToRelative(savePath));
			//	_selectionContext->AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
			//	FileSystem::Scan();
			//	//FileSystemUI::m_currentDirectory = FileSystem::RootDirectory;
			//}
		}
		ImGui::EndPopup();
	}

	ImGui::TableNextColumn();

	ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), GetEntityTypeName(*entity).c_str());

	ImGui::TableNextColumn();
	{
		bool isVisible = entity->IsActive();
		std::string visibilityIcon = isVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		if (ImGui::Button(visibilityIcon.c_str(), { 80, 0 })) {
			isVisible = !isVisible;
		}
		ImGui::PopStyleColor();
	}

	if (open) {
		if ((drawChildren && !isPrefab)) {
			// Caching list to prevent deletion while iterating.
			std::vector<Entity*> childrens = entity->GetChildren();
			for (auto& c : childrens)
				DrawEntityTree(c);
		}

		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
}*/

void SceneHierarchyPanel::DrawStatusBar() {
	if (!Engine::GetCurrentScene()) return;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	float height = ImGui::GetFrameHeight();
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, { m_StatusBarColor.r, m_StatusBarColor.g, m_StatusBarColor.b, m_StatusBarColor.a });
	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
		if (ImGui::BeginMenuBar()) {

			ImGui::Text(m_StatusMessage.c_str());
			ImGui::SameLine();

			const float remainingWidth = ImGui::GetContentRegionAvail().x;
			auto faintLogoTexture = AssetManager::LoadTexture("data/editor/icons/faint-logo.png");
			auto faintSize = Vec2(faintLogoTexture->GetTexture().GetWidth(), faintLogoTexture->GetTexture().GetHeight());
			float sizeDiff = height / faintSize.y;
			float scale = 0.5f;
			const float logoWidth = faintSize.x * sizeDiff;

			std::string version = "dev";

			ImVec2 textSize = ImGui::CalcTextSize(version.c_str());
			ImGui::Dummy({ remainingWidth - (logoWidth / 1.75f) - textSize.x - 8, height });

			ImGui::SameLine();
			ImGui::Text(version.c_str());
			ImGui::SameLine();

			//ImGui::SetCursorPosY(height / 4.0f);
			//ImGui::Image((ImTextureID)(faintLogoTexture->GetID()), ImVec2(logoWidth, height) * scale, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

template<typename T>
void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
{
	if (!_selectionContext->GetComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			_selectionContext->AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}