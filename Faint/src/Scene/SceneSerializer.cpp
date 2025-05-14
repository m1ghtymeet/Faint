#include "hzpch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"
#include "Physics/Physics.h"
#include <Hazel.h>

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Scene/Components/ParentComponent.h"

#include "AssetManagment/Serializable.h"

namespace Faint {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: _scene(scene) {
		
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity) {

		HZ_CORE_ASSERT(entity.HasComponent<NameComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<NameComponent>()) {
			out << YAML::Key << "NameComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<NameComponent>().name;
			auto& id = entity.GetComponent<NameComponent>().id;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::Key << "ID" << YAML::Value << id;

			out << YAML::EndMap; // TagComponent
		}
		if (entity.HasComponent<ParentComponent>()) {
			out << YAML::Key << "ParentComponent";
			out << YAML::BeginMap; // ParentComponent

			auto& comp = entity.GetComponent<ParentComponent>();
			out << YAML::Key << "HasParent" << YAML::Value << comp.HasParent;
			out << YAML::Key << "ParentID" << YAML::Value << comp.ParentID;

			out << YAML::EndMap; // ParentComponent
		}
		if (entity.HasComponent<TransformComponent>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << transform.GetGlobalPosition();
			out << YAML::Key << "Rotation" << YAML::Value << Vec3(transform.GetGlobalRotation().x, transform.GetGlobalRotation().y, transform.GetGlobalRotation().z);
			out << YAML::Key << "Scale" << YAML::Value << transform.GetGlobalScale();

			out << YAML::EndMap; // TransformComponent
		}
		if (entity.HasComponent<CameraComponent>()) {
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			{
				//out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetProjectionType();
				out << YAML::Key << "FOV" << YAML::Value << camera->m_fieldOfView;
				out << YAML::Key << "NearPlane" << YAML::Value << camera->m_nearPlane;
				out << YAML::Key << "FarPlane" << YAML::Value << camera->m_farPlane;
			}
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.fixedAspectRatio;
			
			out << YAML::EndMap; // CameraComponent
		}
		if (entity.HasComponent<MeshRendererComponent>()) {
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap; // MeshRendererComponent

			auto& meshComponent = entity.GetComponent<MeshRendererComponent>();
			out << YAML::Key << "Path" << YAML::Value << meshComponent.ModelPath;

			meshComponent._Model->SerializeDataYaml(out);

			out << YAML::EndMap; // MeshRendererComponent
		}
		if (entity.HasComponent<RigidBodyComponent>()) {
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap; // RigidBodyComponent

			auto& rigidComponent = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "Mass" << YAML::Value << rigidComponent.mass;
			//out << YAML::Key << "LockX" << YAML::Value << rigidComponent.LockX;
			//out << YAML::Key << "LockY" << YAML::Value << rigidComponent.LockY;
			//out << YAML::Key << "LockZ" << YAML::Value << rigidComponent.LockZ;
			//out << YAML::Key << "HasBeenMoved" << YAML::Value << rigidComponent.HasBeenMoved;
			//out << YAML::Key << "QueuedForce" << YAML::Value << rigidComponent.QueuedForce;

			out << YAML::EndMap; // RigidBodyComponent
		}
		if (entity.HasComponent<BoxColliderComponent>()) {
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& boxComponent = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "Size" << YAML::Value << boxComponent.GetSize();
			out << YAML::Key << "IsTrigger" << YAML::Value << boxComponent.IsTrigger;
			//out << YAML::Key << "Box" << YAML::Value << (int)boxComponent.Box.get();

			out << YAML::EndMap; // CameraComponent
		}
		if (entity.HasComponent<LightComponent>()) {
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap; // LightComponent

			auto& component = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Direction" << YAML::Value << component.Direction;
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			out << YAML::Key << "Type" << YAML::Value << static_cast<int>(component.Type);

			out << YAML::EndMap; // LightComponent
		}
		if (entity.HasComponent<NetScriptComponent>()) {
			out << YAML::Key << "NetScriptComponent";
			out << YAML::BeginMap; // NetScriptComponent

			auto& component = entity.GetComponent<NetScriptComponent>();
			out << YAML::Key << "Path" << YAML::Value << component.ScriptPath;
			out << YAML::Key << "Initialized" << YAML::Value << component.Initialized;
			//out << YAML::Key << "Type" << YAML::Value << static_cast<Exposed>(component.ExposedVar);

			out << YAML::EndMap; // NetScriptComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath) {

		YAML::Emitter out;
		out << YAML::BeginMap;
		//std::cout << String::Split(FileSystem::GetFileNameFromPath(filepath), '/') << "\n";
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto view = _scene->Reg().view<NameComponent>();
		for (auto e : view) {
			Entity entity = { e, _scene.get() };
			if (!entity)
				return;
			SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		HZ_CORE_ASSERT(false);
	}

	bool SceneSerializer::Deserialize(const std::string& filepath) {

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .scene file " + filepath + "\n    " + e.what() + "");
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		HZ_CORE_TRACE("Deserializing scene '" + sceneName + "'");

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["NameComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				HZ_CORE_TRACE("Deserialized entity with ID = " + std::to_string(uuid) + ", name = " + name + "");
			
				Entity deserializedEntity = _scene->CreateEntityWithUUID(uuid, name);
				
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{	
					// Gameobjects always have Transforms
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.SetLocalPosition(transformComponent["Position"].as<glm::vec3>());
					transform.SetLocalRotation(Quat(transformComponent["Rotation"].as<glm::vec3>()));
					transform.SetLocalScale(transformComponent["Scale"].as<glm::vec3>());
				}
				
				auto parentComponent = entity["ParentComponent"];
				if (parentComponent)
				{
					auto& parent = deserializedEntity.GetComponent<ParentComponent>();
					//parent.HasParent = parentComponent["HasParent"].as<bool>();
					//if (parent.HasParent)
					//	parent.ParentID = parentComponent["ParentID"].as<int>();
				}
				
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();
					auto camera = cameraComponent["Camera"];
					cc.camera = CreateRef<Camera>();

					//cc.camera->SetProjectionType(camera["ProjectionType"].as<int>());

					cc.camera->m_fieldOfView = camera["FOV"].as<float>();
					cc.camera->m_nearPlane = camera["NearPlane"].as<float>();
					cc.camera->m_farPlane = camera["FarPlane"].as<float>();

					cc.primary = cameraComponent["Primary"].as<bool>();
					cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}
				
				auto meshRendererComponent = entity["MeshRendererComponent"];
				if (meshRendererComponent) {
					auto& meshRenderer = deserializedEntity.AddComponent<MeshRendererComponent>();
					auto modelPath = entity["MeshRendererComponent"]["Path"];
					meshRenderer.ModelPath = modelPath.as<std::string>();
					meshRenderer.LoadModel();

					for (uint32_t i = 0; i < std::size(meshRenderer._Model->GetMeshes()); i++) {
						Ref<Mesh> mesh = meshRenderer._Model->GetMeshes()[i];
						auto meshR = meshRendererComponent["Model"]["Mesh_" + std::to_string(i)];
						//mesh->DeserializeYaml(meshRendererComponent);
						bool loadedMaterialFile = false;
						if (meshR["Material"] && meshR["Material"]["Path"])
						{
							const std::string materialPath = meshR["Material"]["Path"].as<std::string>();
							//std::cout << meshRendererComponent["Model"]["Mesh"] << "\n";
							if (!materialPath.empty())
							{
								Ref<Material> newMaterial = AssetManager::LoadMaterial(materialPath);
								mesh->SetMaterial(newMaterial);
								loadedMaterialFile = true;
							}
						}

						if (!loadedMaterialFile && meshR["Material"])
						{
							Ref<Material> material = mesh->GetMaterial();
							material = CreateRef<Material>();
							//material->Deserialize(meshRendererComponent["Model"]["Material"].as<YAML::Node>());
						}
					}


				}

				auto lightComponent = entity["LightComponent"];
				if (lightComponent) {
					auto& light = deserializedEntity.AddComponent<LightComponent>();
					
					light.Direction = lightComponent["Direction"].as<Vec3>();
					light.Color = lightComponent["Color"].as<Vec3>();
					//light.Type = (LightType)lightComponent["Color"].as<int>();
				}

				auto netComponent = entity["NetScriptComponent"];
				if (netComponent) {
					auto& script = deserializedEntity.AddComponent<NetScriptComponent>();

					script.Initialized = netComponent["Initialized"].as<bool>();
					script.ScriptPath = netComponent["Path"].as<std::string>();
				}
			}
		}

		return true;
	}
	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {

		// Not implemented
		HZ_CORE_ASSERT(false);
		return false;
	}
}