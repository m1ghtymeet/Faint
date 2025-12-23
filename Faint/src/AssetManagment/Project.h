#pragma once
#include <string>
#include <Core/Base.h>
#include "Scene/Scene.h"
#include "AssetManagment/Serializable.h"

namespace YAML {

	class Emitter;
}

enum class BuildMode {
	Raw,
	Mault
};

struct ProjectSettings {
	bool vSync = false;
	bool resizable = true;
	float resolutionScale = 1.0f;
	float gravity = -9.81f;

	BuildMode buildMode = BuildMode::Raw;
	std::string configuration = "debug";
};

namespace Moon {

	struct EditorCameraSettings {
		glm::vec3 position = glm::vec3(0);
		glm::vec3 rotation = glm::vec3(0);
	};

	class Project : public ISerializable {
	public:
		std::string Name;
		std::string Description;
		std::string FullPath;
		std::string AssetDirectory = "Assets";
		Scene* defaultScene = nullptr;

		ProjectSettings settings;
		EditorCameraSettings cameraSettings;

		Project() = default;
		Project(const std::string& name, const std::string& description, const std::string& fullPath, const std::string& defaultScenePath = "");
		~Project();

		void Save();
		void SaveAs(const std::string& fullPath);
		bool Exists() const;

		static std::shared_ptr<Project> Load(const std::string& path);

		json Serialize() override;
		void Deserialize(const json& j) override;
	};
}