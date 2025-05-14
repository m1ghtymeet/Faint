#pragma once
#include "Scene/Entity.h"
#include "FileSystem/FileSystem.h"
#include "AssetManagment/Serializable.h"

#include <string>
#include <vector>
#include <map>

namespace Faint {
	
	struct Prefab : ISerializable {
	public:
		std::string DisplayName;
		std::string Description;
		std::string Path;
		std::vector<Entity> Entities;
		Entity Root;

		static Ref<Prefab> CreatePrefabFromEntity(Entity entity);
		static Ref<Prefab> New(const std::string& path);
		static Ref<Prefab> InstanceInScene();

		Prefab() {
			Path = "";
			Entities = std::vector<Entity>();
		}

		void AddEntity(Entity& entity) {
			Entities.push_back(entity);
		}

		void ReInstance();

		void SaveAs(const std::string& path) {
			this->Path = path;

			FileSystem::BeginWriteFile(path, false);
			FileSystem::WriteLine(Serialize().dump(4));
			FileSystem::EndWriteFile();
		}

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Path);
			SERIALIZE_VAL(DisplayName);
			SERIALIZE_VAL(Description);

			auto entities = std::vector<json>();
			for (Entity e : Entities) {
				entities.push_back(e.Serialize());
			}

			j["Root"] = Root.GetComponent<NameComponent>().name;
			SERIALIZE_VAL_LBL("Entities", entities);

			for (auto& e : j["Entities"]) {
				if (e["NameComponent"]["ID"] == (int)Root.GetComponent<NameComponent>().id)
					e["ParentComponent"]["HasParent"] = false;
			}

			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			return true;
		}
	private:
		void EntityWalker(Entity entity);
	};
}