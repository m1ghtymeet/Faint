#include "CDirectionalLight.h"

Moon::DirectionalLightComp::DirectionalLightComp(Entity& p_owner) :
	LightComponent(p_owner) {

}

std::string Moon::DirectionalLightComp::GetName() {
	return "Directional Light";
}

json Moon::DirectionalLightComp::Serialize() {
	BEGIN_SERIALIZE();
	j["Intensity"] = m_data.strength;
	j["CastShadows"] = m_data.castShadows;
	{
		j["Direction"]["x"] = m_data.direction.x;
		j["Direction"]["y"] = m_data.direction.y;
		j["Direction"]["z"] = m_data.direction.z;
	}
	{
		j["Color"]["x"] = m_data.color.x;
		j["Color"]["y"] = m_data.color.y;
		j["Color"]["z"] = m_data.color.z;
	}
	END_SERIALIZE();
}

void Moon::DirectionalLightComp::Deserialize(const json& j) {
	if (j.contains("Intensity")) m_data.strength = j["Intensity"];
	if (j.contains("CastShadows")) m_data.castShadows = j["CastShadows"];
	if (j.contains("Color"))
		SetColor({ j["Color"]["x"], j["Color"]["y"], j["Color"]["z"] });
	//{
	//	j["Direction"]["x"] = m_data.direction.x;
	//	j["Direction"]["y"] = m_data.direction.y;
	//	j["Direction"]["z"] = m_data.direction.z;
	//}
}
