#include "WrenScriptPanel.h"
#include <Core/OS.h>
#include <Scene/Components/WrenScriptComponent.h>
//#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Util/PlatformUtil.h>
#include "../Panel/FileSystemUI.h"

void WrenScriptPanel::Draw(Faint::Entity& entity)
{	
	Faint::WrenScriptComponent& component = entity.GetComponent<Faint::WrenScriptComponent>();

	ImGui::Text("Script");
	ImGui::SameLine();

	std::string path = component.Path;
	ImGui::Button(path.empty() ? "Create New" : component.Path.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Script"))
		{
			char* file = (char*)payload->Data;

			std::string fullPath = std::string(file, 512);
			std::string a = Faint::FileSystem::AbsoluteToRelative(std::move(fullPath));
			path = "C:\\Users\\Atmosfer-PC\\Documents\\Engine Projects\\Example\\Scripts\\test.wren";

			component.LoadScript(path);
		}
		ImGui::EndDragDropTarget();
	}

	component.Path = path;

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
		if (!component.Path.empty()) {
			if (component.mWrenScript) {
				//Faint::OS::OpenIn(component.mWrenScript->GetFile()->GetAbsolutePath());
			}
		}
		else {

			std::string pathCreation = Faint::FileDialogs::SaveFile("*.wren");

			if (!pathCreation.empty()) {
				if (!Faint::String::EndsWith(pathCreation, ".wren")) {
					pathCreation += ".wren";
				}

				std::string fileName = Faint::String::ToUpper(Faint::FileSystem::GetFileNameFromPath(pathCreation));
				fileName = Faint::String::RemoveWhiteSpace(fileName);

				if (!Faint::String::IsDigit(fileName[0])) {

					Faint::FileSystem::BeginWriteFile(pathCreation, true);
					Faint::FileSystem::WriteLine(TEMPLATE_SCRIPT_FIRST + fileName + TEMPLATE_SCRIPT_SECOND);
					Faint::FileSystem::EndWriteFile();

					path = Faint::FileSystem::AbsoluteToRelative(pathCreation);
					Faint::FileSystem::Scan();
					Faint::FileSystemUI::m_CurrentDirectory = Faint::FileSystem::RootDirectory;
					component.LoadScript(path);
					component.Path = path;
				}
				else {
					HZ_CORE_ERROR("Cannot create script files that starts with a number.");
				}
			}
		}
	}
}
