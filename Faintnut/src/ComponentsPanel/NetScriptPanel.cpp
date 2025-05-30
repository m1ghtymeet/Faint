#include "NetScriptPanel.h"
#include "Core/OS.h"
#include "Core/String.h"
#include "FileSystem/FileSystem.h"
#include "../Panel/FileSystemUI.h"
#include "Util/PlatformUtil.h"
#include "Scripting/ScriptingEngineNet.h"

using namespace Faint;

void NetScriptPanel::Draw(Faint::Entity& entity)
{
	if (!entity.HasComponent<NetScriptComponent>())
		return;

	auto& component = entity.GetComponent<NetScriptComponent>();

	const std::string NET_TEMPLATE_SCRIPT_FIRST = R"(
using System;	
using Faint.Net;

    class )";

	const std::string NET_TEMPLATE_SCRIPT_SECOND = R"( : Entity
{
    public override void OnStart()
    {
        // Called once at the start of the game
    }

    
    public override void OnUpdate(float dt)
    {
        // Called every frame
    }
    
    public override void OnFixedUpdate(float dt)
    {
        // Called every fixed update
    }

    
    public override void OnDestroy()
    {
        // Called at the end of the game fixed update
    }
}
)";

	//BeginComponentTable(.NETSCRIPT, NetScriptComponent);
	//{
		{
			ImGui::Text("Script");
			//ImGui::TableNextColumn();

			std::string path = component.ScriptPath;
			ImGui::Button(path.empty() ? "Create New" : component.ScriptPath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_CSharp"))
				{
					char* file = (char*)payload->Data;

					std::string fullPath = std::string(file, 512);
					path = FileSystem::AbsoluteToRelative(std::move(fullPath));
					//std::cout << fullPath << "\n";
					//path = fullPath;
				}
				ImGui::EndDragDropTarget();
			}

			component.ScriptPath = path;
			if (component.ScriptPath.empty())
				return;

			// Double click on file
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				if (!component.ScriptPath.empty())
				{
					OS::OpenIn(FileSystem::Root + "component.ScriptPath");
				}
				else
				{
					// TODO: Turn into command (Undo/Redo)
					std::string pathCreation = FileDialogs::SaveFile("*.cs");

					if (!pathCreation.empty()) {
						if (!String::EndsWith(pathCreation, ".cs")) {
							pathCreation += ".cs";
						}

						std::string fileName = String::ToUpper(FileSystem::GetFileNameFromPath(pathCreation));
						fileName = String::RemoveWhiteSpace(fileName);

						if (!String::IsDigit(fileName[0]))
						{
							std::cout << "assets/" + FileSystem::AbsoluteToRelative(pathCreation) << "\n";
							FileSystem::BeginWriteFile("assets/" + FileSystem::AbsoluteToRelative(pathCreation));
							FileSystem::WriteLine(NET_TEMPLATE_SCRIPT_FIRST + fileName + NET_TEMPLATE_SCRIPT_SECOND);
							FileSystem::EndWriteFile();

							path = FileSystem::AbsoluteToRelative(pathCreation);
							FileSystem::Scan();
							FileSystemUI::m_CurrentDirectory = FileSystem::RootDirectory;
						}
						else {
							HZ_CORE_ERROR("FileSystem * Cannot create script files that starts with a number.");
						}
					}
				}
			}
			//ImGui::TableNextColumn();

			//ComponentTableReset(component.ScriptPath, "");
		}
		

		std::vector<std::string> detectedExposedVar;
		for (auto& e : ScriptingEngineNet::Get().GetExposedVarForTypes(entity))
		{
			bool found = false;
			for (auto& c : component.ExposedVar)
			{
				if (e.Name == c.Name)
				{
					c.Type = (NetScriptExposedVarType)e.Type;
					c.DefaultValue = e.Value;
					found = true;
				}
			}
			detectedExposedVar.push_back(e.Name);

			if (!found)
			{
				NetScriptExposedVar exposedVar;
				exposedVar.Name = e.Name;
				exposedVar.Value = e.Value;
				exposedVar.DefaultValue = e.Value;
				exposedVar.Type = (NetScriptExposedVarType)e.Type;
				component.ExposedVar.push_back(exposedVar);
			}
		}

		if (ScriptingEngineNet::Get().IsInitialized())
		{
			std::erase_if(component.ExposedVar,
				[&](NetScriptExposedVar& var)
				{
					return std::find(detectedExposedVar.begin(), detectedExposedVar.end(), var.Name) == detectedExposedVar.end();
				});
		}

		for (auto& field : component.ExposedVar)
		{
			ImGui::TableNextColumn();
			{
				ImGui::Text(field.Name.c_str());
				ImGui::SameLine();

				if (field.Type == NetScriptExposedVarType::Float)
				{
					if (!field.Value.has_value())
					{
						if (!field.DefaultValue.has_value())
						{
							field.DefaultValue = 0.0f;
						}

						field.Value = field.DefaultValue;
					}

					float currentValue = std::any_cast<float>(field.Value);
					const std::string sliderName = "##" + field.Name + "solider";
					ImGui::DragFloat(sliderName.c_str(), &currentValue);
					field.Value = currentValue;
				}

				if (field.Type == NetScriptExposedVarType::Double)
				{
					if (!field.Value.has_value())
					{
						if (!field.DefaultValue.has_value())
						{
							field.DefaultValue = 0.0f;
						}

						field.Value = field.DefaultValue;
					}

					float currentValue = (float)std::any_cast<double>(field.Value);
					const std::string sliderName = "##" + field.Name + "solider";
					ImGui::DragFloat(sliderName.c_str(), &currentValue);
					field.Value = (double)currentValue;
				}
			}
		}
	//}
	//EndComponentTable();
}
