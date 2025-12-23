#include "CPointLight.h"

Moon::PointLightComponent::PointLightComponent(Entity& p_owner) :
	LightComponent(p_owner) {

}

void Moon::PointLightComponent::SetRadius(float radius) {
	m_data.radius = radius;
}

std::string Moon::PointLightComponent::GetName() {
	return "Point Light";
}

json Moon::PointLightComponent::Serialize() {
	BEGIN_SERIALIZE();
	j["Radius"] = m_data.radius;
	j["Strength"] = m_data.strength;
	j["CastShadows"] = m_data.castShadows;
	{
		j["Color"]["x"] = m_data.color.x;
		j["Color"]["y"] = m_data.color.y;
		j["Color"]["z"] = m_data.color.z;
	}
	END_SERIALIZE();
}

void Moon::PointLightComponent::Deserialize(const json& j) {

	if (j.contains("Radius")) {
		m_data.radius = j["Radius"];
	}
	if (j.contains("Strength")) {
		m_data.strength = j["Strength"];
	}
	if (j.contains("CastShadows")) {
		m_data.castShadows = j["CastShadows"];
	}
}
