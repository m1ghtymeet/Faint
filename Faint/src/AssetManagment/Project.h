#pragma once
#include <string>
#include "Scene/Scene.h"
#include "AssetManagment/Serializable.h"

namespace YAML {

	class Emitter;
}

namespace Faint {

	struct ProjectSettings {
		bool ShowGizmos = true;
		bool ShowAxis = false;
		bool VSync = false;
		bool resizable = true;
		float ResolutionScale = 1.0f;
		float OutlineRadius = 4.0f;
		float GizmoSize = 0.25f;
		int PhysicsStep = 90;
		int MaxPhysicsSubStep = 32;
		int MaxPhysicsBodies = 4096;
		int MaxPhysicsContactConstraints = 2048;
		int MaxPhysicsBodyPair = 2048;
		Vec4 PrimaryColor = Vec4(0.1f, 0.1f, 0.1f, 1);
		std::string Configuration = "development";

		YAML::Emitter& SerializeYaml(const std::string& path);
		bool DeserializeYaml(const std::string& path);
	};

	class Project : public ISerializable {
	private:
		inline static Ref<Project> m_ActiveProject;
	public:
		std::string Name;
		std::string Description;
		std::string FullPath;
		std::string AssetDirectory;
		std::string ScriptDirectory;

		Ref<Scene> DefaultScene;

		ProjectSettings Settings;

		Project(const std::string& name, const std::string& description, const std::string& fullPath, const std::string& defaultScenePath = "");
		Project();

		void Save();
		void SaveAs(const std::string& fullPath);
		bool FileExist();

		bool SerializeYaml(const std::string& path);
		bool DeserializeYaml(const std::string& path);

		static Ref<Project> New();
		static Ref<Project> New(const std::string& name, const std::string& description, const std::string& fullPath);
		static Ref<Project> Load(const std::string& path);

		void ExportEntitiesToTrenchbroom();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}