#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

using json = nlohmann::json;

#define BEGIN_SERIALIZE() json j;
#define SERIALIZE_VAL_LBL(lbl, v) j[lbl] = v;
#define SERIALIZE_VAL(v) j[#v] = this->v;
#define SERIALIZE_RES_FILE(v) \
		bool validFile = this->v.file != nullptr && this->v.file->Exist(); \
		j["validFile"#v] = validFile; \
		if (validFile) { \
			j["file"#v] = this->v.file->GetRelativePath(); \
		}

#define SERIALIZE_VEC2(v) \
			j[#v]["x"] = v.x; \
			j[#v]["y"] = v.y;

#define SERIALIZE_VEC3(v) \
			SERIALIZE_VEC2(v) \
			j[#v]["z"] = v.z;

#define SERIALIZE_OBJECT(v) j[#v] = v->Serialize();

#define DESERIALIZE_VAL(p) \
	if (j.contains(#p)) { \
		p = j[#p]; \
	}

#define DESERIALIZE_RES_FILE(v) \
	if (j.contains("validFile"#v)) { \
		if (bool validFile = j["validFile"#v]) { \
			const std::string filePath = j["file"#v]; \
		} \
	}

#define DESERIALIZE_VEC2(v, p) \
	p = Vec2(v["x"], v["y"]);

#define DESERIALIZE_VEC3(v, p) \
	p = Vec3(v["x"], v["y"], v["z"]);

#define DESERIALIZE_VEC4(v, p) \
	p = Vec4(v["x"], v["y"], v["z"], v["w"]);

// (v).file = FileSystem::GetFile(filePath); \

#define SERIALIZE_OBJECT_REF_LBL(lbl, v) j[lbl] = v.Serialize();
#define END_SERIALIZE() return j;

//#define BEGIN_SERIALIZE_YAML() \
//	YAML::Emitter out \
//	out << YAML::BeginMap;
//
//#define SERIALIZE_VAL_YAML(name, v) \
//	out << YAML::Key << name << YAML::Value << v;
//
//#define END_SERIALIZE_YAML() return out;

namespace YAML {
	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}
		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
}

class ISerializable {
public:
	virtual json Serialize() = 0;
	virtual bool Deserialize(const json& j) = 0;

	//virtual bool SerializeYaml(const std::string& path) { return false; };
	//virtual bool DeserializeYaml(const std::string& path) { return false; };
};