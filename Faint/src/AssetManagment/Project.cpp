#include "hzpch.h"
#include "Project.h"
#include "Scripting/ScriptingEngineNet.h"

#include <filesystem>
#include <fstream>
#include <streambuf>

namespace Faint {

	Project::Project(const std::string& name, const std::string& description, const std::string& fullPath, const std::string& defaultScenePath) {
		
		this->Name = name;
		this->Description = description;
		this->FullPath = fullPath;

		if (defaultScenePath != "") {
			DefaultScene = CreateRef<Scene>();
			DefaultScene = Scene::Copy(defaultScenePath);
		}

		SaveAs(fullPath);
	}

	Project::Project() {

		this->Name = "";
		this->Description = "";
		this->FullPath = "";
		AssetDirectory = "Assets";
		DefaultScene = CreateRef<Scene>();
	}

	void Project::Save() {

		SerializeYaml(FullPath);
	}

	void Project::SaveAs(const std::string& fullPath) {

		//json j = Serialize();
		//std::string serialized_string = j.dump(4);
		SerializeYaml(fullPath);

		// TODO: Use file interface here...
		// Write to file
		//std::ofstream projectFile;
		//projectFile.open(fullPath);
		//projectFile << serialized_string;
		//projectFile.close();
	}

	bool Project::FileExist()
	{
		return std::filesystem::exists(this->FullPath.c_str());
	}

	Ref<Project> Project::New()
	{
		return CreateRef<Project>();
	}

	Ref<Project> Project::New(const std::string& name, const std::string& description, const std::string& fullPath)
	{
		return CreateRef<Project>(name, description, fullPath);
	}

	Ref<Project> Project::Load(const std::string& path) {

		// Validation
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .project file '{0}'\n		{1}", path, e.what());
			return Ref<Project>();
		}
		if (!data["Project"])
			return Ref<Project>();

		auto projectNode = data["Project"];

		m_ActiveProject = CreateRef<Project>();
		m_ActiveProject->Name = projectNode["Name"].as<std::string>();
		m_ActiveProject->Description = projectNode["Description"].as<std::string>();
		m_ActiveProject->FullPath = path;
		m_ActiveProject->AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		m_ActiveProject->DefaultScene->FullPath = projectNode["StartScene"].as<std::string>();

		m_ActiveProject->Settings = ProjectSettings();

		return m_ActiveProject;
	}

	void Project::ExportEntitiesToTrenchbroom()
	{
		
		for (auto& [name, type] : ScriptingEngineNet::Get().GetBrushEntities())
		{
			std::cout << name << "\n";
		}
	}

	json Project::Serialize()
	{
		return json();
	}

	bool Project::Deserialize(const json& j)
	{
		return false;
	}

	bool Project::SerializeYaml(const std::string& path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Project" << YAML::Value;
			out << YAML::BeginMap;
			{
				out << YAML::Key << "Name" << YAML::Value << Name;
				out << YAML::Key << "Description" << YAML::Value << Description;
				out << YAML::Key << "AssetDirectory" << YAML::Value << AssetDirectory;
				//out << YAML::Key << "ScriptModulePath" << YAML::Value << Description;
				out << YAML::Key << "StartScene" << YAML::Value << DefaultScene->FullPath;
				{
					out << YAML::Key << "Settings" << YAML::Value;
					out << YAML::BeginMap;
					{
						out << YAML::Key << "VSync" << YAML::Value << Settings.VSync;
						out << YAML::Key << "ShowGizmos" << YAML::Value << Settings.ShowGizmos;
						out << YAML::Key << "ShowAxis" << YAML::Value << Settings.ShowAxis;
						out << YAML::Key << "ResolutionScale" << YAML::Value << Settings.ResolutionScale;
						out << YAML::Key << "PhysicsStep" << YAML::Value << Settings.PhysicsStep;
						out << YAML::Key << "MaxPhysicsSubStep" << YAML::Value << Settings.MaxPhysicsSubStep;
						out << YAML::Key << "MaxPhysicsBodies" << YAML::Value << Settings.MaxPhysicsBodies;
						out << YAML::Key << "MaxPhysicsBodyPair" << YAML::Value << Settings.MaxPhysicsBodyPair;
					}
					out << YAML::EndMap;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndMap;
		}

		std::ofstream fout(path);
		fout << out.c_str();
		return true;
	}

	bool Project::DeserializeYaml(const std::string& path)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .project file '{0}'\n		{1}", path, e.what());
			return false;
		}
		if (!data["Project"])
			return false;

		Ref<Project> project = CreateRef<Project>();

		return true;
	}

	YAML::Emitter& ProjectSettings::SerializeYaml(const std::string& path)
	{

		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "ProjectSettings" << YAML::Value;
			out << YAML::BeginMap;
			{
				out << YAML::Key << "VSync" << YAML::Value << VSync;
				out << YAML::Key << "ShowGizmos" << YAML::Value << ShowGizmos;
				out << YAML::Key << "ShowAxis" << YAML::Value << ShowAxis;
				out << YAML::Key << "ResolutionScale" << YAML::Value << ResolutionScale;
				out << YAML::Key << "PhysicsStep" << YAML::Value << PhysicsStep;
				out << YAML::Key << "MaxPhysicsSubStep" << YAML::Value << MaxPhysicsSubStep;
				out << YAML::Key << "MaxPhysicsBodies" << YAML::Value << MaxPhysicsBodies;
				out << YAML::Key << "MaxPhysicsBodyPair" << YAML::Value << MaxPhysicsBodyPair;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		return out;
	}

	bool ProjectSettings::DeserializeYaml(const std::string& path)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .project file '{0}'\n		{1}", path, e.what());
			return false;
		}
		if (!data["Project"])
			return false;
		return true;
	}
}