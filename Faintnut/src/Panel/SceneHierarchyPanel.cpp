#include "SceneHierarchyPanel.h"
#include "Core/Object/Object.h"
#include "Core/String.h"
#include "Core/OS.h"
#include "Util/PlatformUtil.h"
#include "UI/ImUI.h"
#include "UI/PopupHelper.h"
#include "Engine.h"

// Resources
#include "AssetManagment/ModelLoader.h"
#include "AssetManagment/FontAwesome5.h"

#include <Hazel.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/imgui_stdlib.h>
#include <filesystem>

// Components
#include "Scene/Components.h"
#include "Scene/Components/ParentComponent.h"
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

ImFont* normalFont;

std::string GetEntityTypeName(const Entity& entity) {
	
	std::string entityTypeName = "";

	if (entity.HasComponent<CameraComponent>()) {
		entityTypeName = "Camera";
	}

	if (entity.HasComponent<RigidBodyComponent>()) {
		entityTypeName = "Rigidbody";
	}

	if (entity.HasComponent<PrefabComponent>()) {
		entityTypeName = "Prefab";
		return entityTypeName;
	}

	if (entity.HasComponent<MeshRendererComponent>()) {
		entityTypeName = "Model";
	}

	if (entity.HasComponent<LightComponent>()) {
		entityTypeName = "Light";
	}

	return entityTypeName;
}

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) {
	SetContext(context);

	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	//ImFontConfig iconsConfigBold;
	//float fontSize = 20.0f;
	//iconsConfigBold.MergeMode = true;
	//normalFont = io.Fonts->AddFontFromFileTTF("res/Fonts/fa-regular-400.ttf", fontSize, &iconsConfigBold, icon_ranges);
	//iconsConfigBold.MergeMode = false;
}

void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
	_context = context;
	_selectionContext = {};
}

void SceneHierarchyPanel::OnImGuiRender() {

	//auto view = _context->Reg().view<NameComponent>();
	//for (auto e : view) {
	//	Entity entity{ e, _context.get() };
	//	DrawGameObjectNode(entity);
	//}

	if (!_context) return;

	if (ImGui::BeginDragDropTarget()) {
		//auto& gBuffer = Engine::GetCurrentScene()->m_sceneRenderer;
		
	}

	if (ImGui::Begin("Scene Hierarchy")) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
		ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
		ImGui::PopStyleVar();

		ImGui::SameLine();

		if (UI::PrimaryButton("Add Entity", { ImGui::GetContentRegionAvail().x, 0 })) {
			ImGui::OpenPopup("create_entity_popup");
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
		if (ImGui::BeginPopup("create_entity_new_popup")) {
			ImGui::BeginChild("entity_child", ImVec2(442, 442), ImGuiChildFlags_AlwaysUseWindowPadding);
			
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
			ImGui::PopItemWidth();

			ImGui::EndChild();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginPopup("create_entity_popup")) {
			Entity entity;
			if (ImGui::MenuItem("Empty")) {
				entity = _context->CreateEntity("Empty");
			}

			if (ImGui::BeginMenu("2D")) {
				if (ImGui::MenuItem("Sprite")) {
					entity = _context->CreateEntity("Camera");
					entity.AddComponent<SpriteComponent>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("3D")) {
				if (ImGui::MenuItem("Camera")) {
					entity = _context->CreateEntity("Camera");
					entity.AddComponent<CameraComponent>();
				}
				if (ImGui::MenuItem("Light")) {
					entity = _context->CreateEntity("Light");
					entity.AddComponent<LightComponent>();
				}
				if (ImGui::MenuItem("Box Collider")) {
					entity = _context->CreateEntity("Box Collider");
					entity.AddComponent<RigidBodyComponent>();
					entity.AddComponent<BoxColliderComponent>();
				}
				if (ImGui::MenuItem("Sphere Collider")) {
					entity = _context->CreateEntity("Sphere Collider");
					entity.AddComponent<RigidBodyComponent>();
					entity.AddComponent<SphereColliderComponent>();
				}
				if (ImGui::MenuItem("Character Controller")) {
					entity = _context->CreateEntity("Character Controller");
					entity.AddComponent<CharacterControllerComponent>();
				}
				if (ImGui::MenuItem("Net Script")) {
					entity = _context->CreateEntity("Net Script");
					entity.AddComponent<NetScriptComponent>();
				}
				if (ImGui::MenuItem("Wren Script")) {
					entity = _context->CreateEntity("Wren Script");
					entity.AddComponent<WrenScriptComponent>();
				}
				if (ImGui::MenuItem("Audio")) {
					entity = _context->CreateEntity("Audio");
					entity.AddComponent<AudioComponent>();
				}
				ImGui::EndMenu();
			}

			if (entity.IsValid()) {
				if (_selectionContext.IsValid()) {
					_selectionContext.AddChild(entity);
				}
				else {

				}
				_selectionContext = entity;
			}

			ImGui::EndPopup();
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			_selectionContext = {};

		// Draw a tree of entities
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
		if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
		{
			if (ImGui::BeginTable("entity_table", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("   Name", ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Script", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Visibility   ", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				//ImGui::TableHeadersRow();
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

				// Build list of valid entity to display
				std::vector<Entity> entitiesToDisplay;
				if (searchQuery.empty()) {
					entitiesToDisplay = _context->GetAllEntities();
				}
				else {
					auto view = _context->Reg().view<NameComponent>();
					for (auto& e : view)
					{
						auto& nameComponent = view.get<NameComponent>(e);
						if (String::RemoveWhiteSpace(String::ToUpper(nameComponent.name)).find(String::RemoveWhiteSpace(String::ToUpper(searchQuery))) != std::string::npos)
						{
							entitiesToDisplay.push_back({ e, _context.get() });
						}
					}
				}

				// Right-click on black space
				if (ImGui::BeginPopupContextWindow(0, 1)) {
					if (ImGui::MenuItem("Create Empty Entity"))
						_context->CreateEntity("Empty Entity");
					ImGui::EndPopup();
				}

				// Display valid entities
				for (Entity e : entitiesToDisplay)
				{
					// Draw all entity without parents.
					bool displayAllHierarchy = searchQuery.empty();
					if ((displayAllHierarchy && !e.GetComponent<ParentComponent>().HasParent) || !displayAllHierarchy)
					{
						//ImGui::PushFont(normalFont);

						// Recursively draw childrens if not searching
						const std::string entityName = e.GetComponent<NameComponent>().name;

						DrawEntityTree(e, displayAllHierarchy);

						//ImGui::PopFont();
					}
				}
				ImGui::PopStyleVar();
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab")) {
				char* file = (char*)payload->Data;
				std::string fullPath = std::string(file, 256);
				std::string relPath = FileSystem::AbsoluteToRelative(fullPath);
				auto newPrefabInstance = Prefab::New(relPath);
				newPrefabInstance->Root.GetComponent<PrefabComponent>().SetPrefab(newPrefabInstance);
				newPrefabInstance->Root.GetComponent<NameComponent>().isPrefab = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	// Deleted entity queue
	if (QueueDeletion.GetHandle() != -1) {
		Engine::GetCurrentScene()->DestroyEntity(QueueDeletion);

		_selectionContext = {};

		QueueDeletion = Entity{ (entt::entity)-1, _context.get() };
	}
	ImGui::End();

	ImGui::Begin("Properties");
	if (_selectionContext) {
		DrawComponents(_selectionContext);

		auto view = _context->Reg().view<NameComponent>();
		for (auto e : view) {
			Entity entity{ e, _context.get() };
		
			for (auto&& [componentTypeId, storage] : _context->Reg().storage()) {
		
				entt::type_info componentType = storage.type();
		
				entt::entity entityId = static_cast<entt::entity>(entity.GetHandle());
				if (storage.contains(entityId)) {
					entt::meta_type type = entt::resolve(componentType);
					entt::meta_any component = type.from_void(storage.value(entityId));
		
					//ComponentTypeTrait typeTraits = type.traits<ComponentTypeTrait>();
				//// Component not exposed as an inspector panel
				//if ((typeTraits & ComponentTypeTrait::InspectorExposed) == ComponentTypeTrait::None) {
				//	continue;
				//}
				}
			}
		}
	}
	ImGui::End();
}
void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
	_selectionContext = entity;
	EntityChanged = true;
}
void SceneHierarchyPanel::DrawGameObjectNode(Entity entity) {

	std::string tagName = entity.GetComponent<NameComponent>().name;

	ImGuiTreeNodeFlags flags = ((_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	
	bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagName.c_str());
	if (ImGui::IsItemClicked()) {
		_selectionContext = entity;
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Delete GameObject"))
			entityDeleted = true;

		ImGui::EndPopup();
	}

	//if (opened) {
	//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth;
	//	opened = ImGui::TreeNodeEx((void*)9817239, flags, tagName.c_str());
	//	if (opened)
	//		ImGui::TreePop();
	//	ImGui::TreePop();
	//}

	if (entityDeleted) {
		_context->DestroyEntity(entity);
		if (_selectionContext == entity)
			_selectionContext = {};
	}
}
static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {

	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight + 1.0f };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();
	
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.24f, 0.7f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.07f, 0.24f, 0.7f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.05f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();
}

template<typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity& entity, UIFunction ui) {
	
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity.HasComponent<T>()) {

		ImGui::PushID(name.c_str());

		auto& component = entity.GetComponent<T>();
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

		if (removeComponent)
			entity.RemoveComponent<T>();

		ImGui::PopID();
	}
}

void SceneHierarchyPanel::DrawComponents(Entity& entity) {

	if (entity.HasComponent<NameComponent>())
	{
		auto& tagName = entity.GetComponent<NameComponent>().name;
		
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), tagName.c_str());
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
			tagName = std::string(buffer);
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);
	

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponent");

	if (ImGui::BeginPopup("AddComponent"))
	{
		DisplayAddComponentEntry<CameraComponent>("Camera");
		DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
		DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
		DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
		DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller");
		DisplayAddComponentEntry<RigidBodyComponent>("RigidBody");
		DisplayAddComponentEntry<NetScriptComponent>("Net Script");
		DisplayAddComponentEntry<WrenScriptComponent>("Wren Script");
		DisplayAddComponentEntry<LuaScriptComponent>("Lua Script");
		DisplayAddComponentEntry<SpriteComponent>("Sprite");
		DisplayAddComponentEntry<AudioComponent>("Audio");
		DisplayAddComponentEntry<LightComponent>("Light");
		DisplayAddComponentEntry<TextComponent>("Text");
		DisplayAddComponentEntry<TextBlitterComponent>("Text Blitter");
		DisplayAddComponentEntry<SkyComponent>("Sky");

		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();

	DrawComponent<TransformComponent>("Transform", entity, [&entity](TransformComponent& component) {
		DrawVec3Control("Position", component.GetTransform().m_localPosition);
		
		Quat currentRotation = component.GetLocalRotation();
		Vec3 eulerDegreesOld = glm::degrees(glm::eulerAngles(currentRotation));
		DrawVec3Control("Rotation", eulerDegreesOld);

		DrawVec3Control("Scale", component.GetTransform().m_localScale);
	});

	DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
		auto& camera = component.camera;

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
		/*if (component.Texture) ImGui::ImageButton((ImTextureID)component.Texture->GetID(), ImVec2(80.0f, 80.0f));
		else ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Texture")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path texturePath = std::filesystem::path(s_assetPath) / path;
				component.Texture = Texture2D::Create(texturePath.string());
			}
			ImGui::EndDragDropTarget();
		}*/

		std::string label = "None";

		const bool isModelNone = component._Model == nullptr;
		if (!isModelNone) {
			label = component._Model->Name;
		}

		if (EntityChanged) {
			_modelInspector = CreateScope<ModelResourceInspector>(component._Model);
			EntityChanged = false;
		}

		if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			if (!isModelNone) {
				if (!_expanded) {
					_modelInspector = CreateScope<ModelResourceInspector>(component._Model);
				}
				_expanded = !_expanded;
			}
		}

		if (_expanded) {
			_modelInspector->Draw();
		}

		bool shouldConvert = false;
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model")) {
				char* file = (char*)payload->Data;
				std::string fullPath = std::string(file, 256);
				//fullPath = FileSystem::AbsoluteToRelative(fullPath);

				if (String::EndsWith(fullPath, ".mesh")) {

					component.ModelPath = fullPath;
					component._Model = AssetManager::LoadModel(fullPath);
				}
				else {
					component.ModelPath = fullPath;
					component.LoadModel();
				
					_importedModelPath = fullPath;

					auto loader = ModelLoader();
					auto modelResource = loader.LoadModel(fullPath, true);
					shouldConvert = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (PopupHelper::ConfirmationDialog("##ConvertAsset", "Convert Asset")) {

			// Convert to disk
			auto loader = ModelLoader();
			Ref<Model> modelResource = loader.LoadModel(_importedModelPath);
			json serializeData = modelResource->SerializeData();

			//std::cout << String::Split(_importedModelPath, '.')[0] << "\n";
			const std::string exportedMeshPath = String::Split(_importedModelPath, '.')[0] + ".mesh";
			FileSystem::BeginWriteFile(exportedMeshPath, true);
			FileSystem::WriteLine(serializeData.dump());
			FileSystem::EndWriteFile();

			
			component.ModelPath = exportedMeshPath;
			component._Model = modelResource;
		}

		if (shouldConvert) {
			PopupHelper::OpenPopup("##ConvertAsset");
		}
	});

	DrawComponent<BoxColliderComponent>("Box Collider", entity, [&entity](auto& component) {
		DrawVec3Control("Size", component.halfExtents);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100);
		ImGui::Text("Is Trigger");
		ImGui::NextColumn();
		component.IsTrigger = UI::CheckBox("Is Trigger", component.IsTrigger);
		ImGui::Columns(1);
	});

	DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [&entity](auto& component) {

		UI::FloatSlider("Radius", component.Radius, 0.0f, 1.0f);
	});

	DrawComponent<CharacterControllerComponent>("Character Controller", entity, [&entity](auto& component) {

		BeginComponentTable(CHARACTER CONTROLLER, CharacterControllerComponent);
		{
			{
				ImGui::Text("Friction");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##Friction", &component.Friction, 0.00f, 0.1f, 100.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.Friction, 0.5f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Max Slope Angle");
				ImGui::TableNextColumn();

				ImGui::DragFloat("##MaxSlopeAngle", &component.MaxSlopeAngle, 0.01f, 0.1f, 90.0f);
				ImGui::TableNextColumn();
				ComponentTableReset(component.MaxSlopeAngle, 0.45f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Auto Stepping");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##AutoStepping", &component.AutoStepping);
				ImGui::TableNextColumn();
				ComponentTableReset(component.AutoStepping, true);
			}
			if (component.AutoStepping)
			{
				ImGui::TableNextColumn();
				{
					ImGui::Text("Stick to floor step down");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##StickToFloorStepDown", &component.StickToFloorStepDown.y, -10.0f, 0.01, 0.0f);
					ImGui::TableNextColumn();
					ComponentTableReset(component.StickToFloorStepDown.y, -0.5f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Step down extra");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##StepDownExtra", &component.StepDownExtra.y, -10.0f, 0.01, 0.0f);
					ImGui::TableNextColumn();
					ComponentTableReset(component.StepDownExtra.y, 0.0f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Step up");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##StepUp", &component.SteppingStepUp.y, 0.0f, 0.01, 10.0f);
					ImGui::TableNextColumn();
					ComponentTableReset(component.SteppingStepUp.y, 0.4f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Step distance");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##StepDistance", &component.SteppingForwardDistance, 0.0f, 0.01f, 10.0f);
					ImGui::TableNextColumn();
					ComponentTableReset(component.SteppingForwardDistance, 0.250f)
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Step min distance");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##StepMinDistance", &component.SteppingMinDistance, 0.0f, 0.01f, component.SteppingForwardDistance);
					ImGui::TableNextColumn();
					ComponentTableReset(component.SteppingMinDistance, 0.125f)
				}
			}
		}
		EndComponentTable();
	});

	DrawComponent<RigidBodyComponent>("RigidBody", entity, [&entity](auto& component) {
		{
			ImGui::Text("Mass");
			ImGui::SameLine();

			ImGui::DragFloat("##Mass", &component.mass, 0.01f, 0.1f);
			component.mass = std::max(component.mass, 0.0f);
		}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock X axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##lockx", &component.LockX);
		//}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock Y axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##locky", &component.LockY);
		//}
		//ImGui::NextColumn();
		//{
		//	ImGui::Text("Lock Z axis");
		//	ImGui::SameLine();
		//
		//	ImGui::Checkbox("##lockz", &component.LockZ);
		//}
	});

	DrawComponent<NetScriptComponent>("Net Script", entity, [&entity](auto& component) {

		netPanel->Draw(entity);
	});

	DrawComponent<LuaScriptComponent>("Lua Script", entity, [&entity](auto& component) {

		//component.path = "";
		
	});

	DrawComponent<LightComponent>("Point Light", entity, [&entity](auto& component) {
		
		BeginComponentTable(LIGHT, LightComponent);
		{
			{
				ImGui::Text("Color");

				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##lightColor", &component.Color.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				ComponentTableReset(component.Color, Vec3(1, 1, 1));
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Strength");

				ImGui::TableNextColumn();
				ImGui::DragFloat("Strength", &component.Strength, 0.1f, 0.0f, 100.0f);

				ImGui::TableNextColumn();
				ComponentTableReset(component.Strength, 1.f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Type");

				ImGui::TableNextColumn();
				const char* lightTypes[] = { "Point", "Directional", "Spot" };
				ComponentDropDown(lightTypes, LightType, component.Type);

				ImGui::TableNextColumn();
				ComponentTableReset(component.Type, LightType::Point);
			}
			ImGui::TableNextColumn();

			if (component.Type == Faint::LightType::Point) {
				{
					ImGui::Text("Radius");

					ImGui::TableNextColumn();
					ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.0f, 100.0f);

					ImGui::TableNextColumn();
					ComponentTableReset(component.Radius, 1.f);
				}
			}
			else if (component.Type == Faint::LightType::Directional)
			{
				{
					ImGui::Text("Is Volumetric");

					ImGui::TableNextColumn();
					ImGui::Checkbox("##Volumetric", &component.IsVolumetric);

					ImGui::TableNextColumn();
					ComponentTableReset(component.IsVolumetric, false);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Sync Direction with sky");

					ImGui::TableNextColumn();
					ImGui::Checkbox("##SyncSky", &component.SyncDirectionWithSky);

					ImGui::TableNextColumn();
					ComponentTableReset(component.SyncDirectionWithSky, false);

					// Light direction is only useful if it is not overriden by the procedural sky direction
					if (!component.SyncDirectionWithSky)
					{
						ImGui::TableNextColumn();
						{
							ImGui::Text("Direction");

							ImGui::TableNextColumn();
							//ImGuiHelper::DrawVec3("Direction", &component.Direction);

							ImGui::TableNextColumn();
							ComponentTableReset(component.Direction, Vec3(0, -1, 0));
						}
					}
				}
			}
			else if (component.Type == Faint::LightType::Spot)
			{
				{
					ImGui::Text("Cutoff");
					ImGui::TableNextColumn();

					ImGui::DragFloat("##cutoff", &component.Cutoff, 1.0f, 0.0f, 360.0f);
					ImGui::TableNextColumn();

					// Clamp inner angle so it doesnt exceed outer angle.
					component.Cutoff = glm::min(component.Cutoff, component.Outercutoff);

					ComponentTableReset(component.Cutoff, 12.5f);
					ImGui::TableNextColumn();
				}

				{
					ImGui::Text("Outer Cutoff");
					ImGui::TableNextColumn();

					ImGui::DragFloat("##outercutoff", &component.Outercutoff, 1.0f, component.Cutoff, 360.0f);
					ImGui::TableNextColumn();

					ComponentTableReset(component.Outercutoff, 30.0f);
					ImGui::TableNextColumn();
				}
			}
		}
		EndComponentTable();
	});

	DrawComponent<SpriteComponent>("Sprite", entity, [&entity](auto& component) {


	});

	DrawComponent<AudioComponent>("Audio", entity, [&entity](auto& component) {

		BeginComponentTable(AUDIO, AudioComponent);
		{
			{
				ImGui::Text("Audio File");
				ImGui::TableNextColumn();

				std::string path = component.FilePath;
				ImGui::Button(path.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0 });
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Audio")) {

						char* file = (char*)payload->Data;
						std::string fullPath = std::string(file, 256);
						path = FileSystem::AbsoluteToRelative(fullPath);
						component.FilePath = path;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::TableNextColumn();

				ComponentTableReset(component.FilePath, "");
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Playing");
				ImGui::TableNextColumn();

				UI::CheckBox("##Player", component.IsPlaying);
				ImGui::TableNextColumn();

				ComponentTableReset(component.IsPlaying, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Loop");
				ImGui::TableNextColumn();

				UI::CheckBox("##Loop", component.Loop);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Loop, false);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Volume");
				ImGui::TableNextColumn();

				UI::FloatSlider("##Volume", component.Volume, 0.0f, 2.0f, 0.001f);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Volume, 1.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Playback Speed");
				ImGui::TableNextColumn();

				UI::FloatSlider("##PlaybackSpeed", component.PlaybackSpeed, 0.0001f, 1.0f, 0.01f);
				ImGui::TableNextColumn();

				ComponentTableReset(component.PlaybackSpeed, 1.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Pan");
				ImGui::TableNextColumn();

				UI::FloatSlider("##Pan", component.Pan, -1.0f, 1.0f, 0.01f);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Pan, 0.0f);
			}
			ImGui::TableNextColumn();
			{
				ImGui::Text("Spatialized");
				ImGui::TableNextColumn();

				UI::CheckBox("##Spatialized", component.Spatialized);
				ImGui::TableNextColumn();

				ComponentTableReset(component.Spatialized, false);
			}

			if (component.Spatialized) {
				ImGui::TableNextColumn();
				{
					ImGui::Text("Min Distance");
					ImGui::TableNextColumn();

					UI::FloatSlider("##minDistance", component.MinDistance, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component.MinDistance, 1.0f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Max Distance");
					ImGui::TableNextColumn();

					UI::FloatSlider("##maxDistance", component.MaxDistance, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component.MaxDistance, 10.0f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Text("Attenuation Factor");
					ImGui::TableNextColumn();

					UI::FloatSlider("##attenuationFactor", component.AttenuationFactor, 0.0f, 1.0f, 0.001f);
					ImGui::TableNextColumn();

					ComponentTableReset(component.AttenuationFactor, 1.0f);
				}
			}
		}
		EndComponentTable();
	});

	DrawComponent<WrenScriptComponent>("Wren Script", entity, [&entity](auto& component) {

		scriptPanel->Draw(entity);
	});

	DrawComponent<TextComponent>("Text", entity, [&entity](auto& component) {

		ImGui::InputTextMultiline("Text String", &component.TextString);
		ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		ImGui::DragFloat("Kerning", &component.Kerning, 0.025f);
		ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.025f);
	});

	DrawComponent<TextBlitterComponent>("Text Blitter", entity, [&entity](auto& component) {

		ImGui::InputTextMultiline("Text", &component.Text);
		ImGui::DragInt("Location X", &component.LocationX, 0.025f);
		ImGui::DragInt("Location Y", &component.LocationY, 0.025f);
		ImGui::DragFloat("Scale", &component.Scale, 0.25f);
	});

	DrawComponent<SkyComponent>("Sky", entity, [&entity](auto& component) {

		
	});
}

void SceneHierarchyPanel::DrawEntityTree(Entity entity, bool drawChildren) {

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
	
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

	NameComponent& nameComponent = entity.GetComponent<NameComponent>();
	ParentComponent& parent = entity.GetComponent<ParentComponent>();

	std::string name = nameComponent.name;

	if (_selectionContext == entity)
		base_flags |= ImGuiTreeNodeFlags_Selected;

	ImGui::TableNextColumn();

	// If has no children draw tree node leaf
	bool isPrefab = entity.HasComponent<PrefabComponent>();
	if (parent.Children.size() <= 0 || isPrefab || !drawChildren) {
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (nameComponent.isPrefab && entity.HasComponent<PrefabComponent>()) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	}
	else if (entity.HasComponent<BSPBrushComponent>()) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
	}

	if (!m_IsRenaming && entity.GetScene()->EntityIsParent(_selectionContext, entity)) {
		ImGui::SetNextItemOpen(true);
	}

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

		//fg->AddRectFilled(ImVec2(cursorPosition.x + 20.0f, cursorPosition.y + 4.0f), ImVec2())
	}

	if (m_IsRenaming) {
		if (_selectionContext == entity) {
			ImGui::SetCursorPosY(cursorPos.y);
			ImGui::Indent();
			ImGui::InputText("##renamingEntity", &name);
			ImGui::Unindent();
			if (Input::KeyPressed(Key::Enter)) {
				nameComponent.name = name;
				m_IsRenaming = false;
			}
		}
	}

	bool isDragging = false;
	if (nameComponent.isPrefab && entity.HasComponent<PrefabComponent>()) {
		ImGui::PopStyleColor();
	}

	if (!m_IsRenaming && ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("ENTITY", (void*)&entity, sizeof(Entity));
		ImGui::Text(name.c_str());
		ImGui::EndDragDropSource();
	}

	if (!isPrefab && ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
			Entity payload_entity = *(const Entity*)payload->Data;

			// Check if entity is already parent.
			ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
			if (!payload_entity.EntityContainsItself(payload_entity, entity) && parentPayload.Parent != entity && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0) {
				if (parentPayload.HasParent) {
					// Erase remove idiom.
					ParentComponent& childOfParent = parentPayload.Parent.GetComponent<ParentComponent>();
					childOfParent.Children.erase(std::remove(childOfParent.Children.begin(), childOfParent.Children.end(), payload_entity), childOfParent.Children.end());
				}

				parentPayload.Parent = entity;
				parentPayload.HasParent = true;
				parent.Children.push_back(payload_entity);
			}
		}
		else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CSharp"))
		{
			char* file = (char*)payload->Data;

			std::string fullPath = std::string(file, 512);
			std::string path = FileSystem::AbsoluteToRelative(std::move(fullPath));

			if (entity.HasComponent<NetScriptComponent>()) {
				entity.GetComponent<NetScriptComponent>().ScriptPath = path;
			} else {
				entity.AddComponent<NetScriptComponent>().ScriptPath = path;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (!isDragging && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		// We selected another another that ew weren't renaming
		if (_selectionContext != entity) {
			m_IsRenaming = false;
		}

		_selectionContext = entity;
		EntityChanged = true;
	}

	if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)))
	{
		m_IsRenaming = true;
	}

	if (ImGui::BeginPopupContextItem()) {
		
		_selectionContext = entity;

		if (_selectionContext.HasComponent<CameraComponent>()) {
			// Moves the editor camera to the camera position and direction.
			if (ImGui::Selectable("Focus camera")) {
				
			}
			ImGui::Separator();
		}

		if (ImGui::Selectable("Remove")) {
			QueueDeletion = entity;
		}

		if (entity.GetComponent<ParentComponent>().HasParent && ImGui::Selectable("Move to root")) {
			auto& parentComp = _selectionContext.GetComponent<ParentComponent>();
			if (parentComp.HasParent) {
				auto& parentParentComp = parentComp.Parent.GetComponent<ParentComponent>();
				parentParentComp.RemoveChildren(entity);
				parentComp.HasParent = false;
			}
		}

		if (!isPrefab && ImGui::Selectable("Save entity as a new prefab")) {
			Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(_selectionContext);
			std::string savePath = FileDialogs::SaveFile("*.prefab");
			if (!String::EndsWith(savePath, ".prefab")) {
				savePath += ".prefab";
			}

			if (!savePath.empty()) {
				newPrefab->SaveAs(FileSystem::AbsoluteToRelative(savePath));
				_selectionContext.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
				FileSystem::Scan();
				//FileSystemUI::m_currentDirectory = FileSystem::RootDirectory;
			}
		}
		ImGui::EndPopup();
	}

	ImGui::TableNextColumn();

	ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), GetEntityTypeName(entity).c_str());

	ImGui::TableNextColumn();
	{
		bool hasScript = entity.HasComponent<NetScriptComponent>();
		if (hasScript) {
			std::string scrollIcon = std::string(ICON_FA_SCROLL) + "##" + name;
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
			if (ImGui::Button(scrollIcon.c_str(), {40, 0}))
			{
				auto& scriptComponent = entity.GetComponent<NetScriptComponent>();
				if (!scriptComponent.ScriptPath.empty() && FileSystem::FileExists(scriptComponent.ScriptPath))
				{
					OS::OpenIn(FileSystem::RelativeToAbsolute(scriptComponent.ScriptPath));
				}
			}
			ImGui::PopStyleColor();
		}
	}

	ImGui::TableNextColumn();
	{
		bool& isVisible = entity.GetComponent<VisibilityComponent>().Visible;
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
			std::vector<Entity> childrens = parent.Children;
			for (auto& c : childrens)
				DrawEntityTree(c);
		}

		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
}

void SceneHierarchyPanel::DrawStatusBar()
{
	if (!_context) return;

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
			auto faintLogoTexture = TextureManager::Get()->GetTexture("data/editor/icons/faint-logo.png");
			auto faintSize = Vec2(faintLogoTexture->GetWidth(), faintLogoTexture->GetHeight());
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
	if (!_selectionContext.HasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			_selectionContext.AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}