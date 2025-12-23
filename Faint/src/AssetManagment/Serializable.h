#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

/* =============== JSON =============== */
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
	p = glm::vec2(v["x"], v["y"]);

#define DESERIALIZE_VEC3(v, p) \
	p = glm::vec3(v["x"], v["y"], v["z"]);

#define DESERIALIZE_VEC4(v, p) \
	p = glm::vec4(v["x"], v["y"], v["z"], v["w"]);

#define SERIALIZE_OBJECT_REF_LBL(lbl, v) j[lbl] = v.Serialize();
#define END_SERIALIZE() return j;


class ISerializable {
public:
	virtual ~ISerializable() = default;

	virtual json Serialize() = 0;
	virtual void Deserialize(const json& j) = 0;

	//virtual void SerializeCBA(std::vector<uint8_t>& data) const = 0;
	//virtual void DeserializeCBA(std::vector<uint8_t>& data, size_t size) = 0;
};