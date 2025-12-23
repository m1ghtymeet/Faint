#include "Project.h"
#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>

#include <filesystem>
#include <fstream>

Moon::Project::Project(const std::string& name, const std::string& description, const std::string& fullPath, const std::string& defaultScenePath) {
	Name = name;
	Description = description;
	FullPath = fullPath;
	defaultScene = new Scene();
	if (defaultScenePath != "") {
		defaultScene->FullPath = defaultScenePath;
	}
	SaveAs(fullPath);
}

Moon::Project::~Project()
{
}

void Moon::Project::Save() {
	if (!FullPath.empty()) SaveAs(FullPath);
	else HZ_CORE_ERROR("Project path is empty, cannot save project.");
}

void Moon::Project::SaveAs(const std::string& fullPath) {
	FullPath = fullPath;
	json j = Serialize();
	std::ofstream file(fullPath);
	if (file.is_open()) {
		file << j.dump(4);
		file.close();
		HZ_CORE_INFO("Project saved to: {}", fullPath);
	}
	else {
		HZ_CORE_ERROR("Failed to save project to: {}", fullPath);
	}
}

bool Moon::Project::Exists() const {
	return !FullPath.empty() && std::filesystem::exists(FullPath);
}

std::shared_ptr<Moon::Project> Moon::Project::Load(const std::string& path) {
	std::ifstream file(path);
	if (!file) return nullptr;
	json j;
	try { file >> j; }
	catch (...) { return nullptr; }
	auto project = std::make_shared<Project>();
	project->Deserialize(j);
	project->FullPath = path;
	return project;
}

json Moon::Project::Serialize() {
	BEGIN_SERIALIZE();
	j["name"] = Name;
	j["description"] = Description;
	j["assetDirectory"] = AssetDirectory;
	j["startScene"] = FileSystem::AbsoluteToRelative(defaultScene->FullPath);
	j["settings"]["vSync"] = settings.vSync;
	j["settings"]["gravity"] = settings.gravity;
	j["settings"]["resolutionScale"] = settings.resolutionScale;
	j["settings"]["buildMode"] = settings.buildMode;
	j["settings"]["configuration"] = settings.configuration;
	j["cameraSettings"]["position"] = { cameraSettings.position.x, cameraSettings.position.y, cameraSettings.position.z };
	j["cameraSettings"]["rotation"] = { cameraSettings.rotation.x, cameraSettings.rotation.y, cameraSettings.rotation.z };
	END_SERIALIZE();
}

void Moon::Project::Deserialize(const json& j) {
	Name = j.value("name", "");
	Description = j.value("description", "");
	AssetDirectory = j.value("assetDirectory", "");
	if (j.contains("startScene")) {
		if (!defaultScene) defaultScene = new Scene();
		defaultScene->FullPath = FileSystem::RelativeToAbsolute(j["startScene"]);
	}
	if (j.contains("settings")) {
		auto settingsJson = j["settings"];
		if (settingsJson["vSync"]) settings.vSync = settingsJson["vSync"];
		//if (settingsJson["gravity"]) settings.gravity = float(settingsJson["gravity"]);
		//if (settingsJson["resolutionScale"]) settings.resolutionScale = settingsJson["resolutionScale"];
		//if (settingsJson["buildMode"]) settings.buildMode = settingsJson["buildMode"];
		//if (settingsJson["configuration"]) settings.configuration = settingsJson["configuration"];
	}
}