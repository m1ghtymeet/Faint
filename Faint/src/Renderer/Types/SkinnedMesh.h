#pragma once
#include "Core/Base.h"

#include "Common/Types.h"
#include "AssetManagment/Serializable.h"
#include "Mesh.h"
#include <vector>

namespace YAML {
	class Emitter;
}

namespace Faint {
	struct Model : public Resource, ISerializable {
	private:
		std::vector<Ref<Mesh>> m_meshes;
	public:
		Model();
		Model(const std::string path);
		~Model() = default;

		std::string Name = "None";

		void AddMesh(Ref<Mesh> mesh);
		std::vector<Ref<Mesh>>& GetMeshes();

		json SerializeData();
		bool SerializeDataYaml(YAML::Emitter& out);
		bool DeserializeDataYaml();
		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}