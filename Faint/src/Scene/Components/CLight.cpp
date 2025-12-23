#include "CLight.h"
#include <Scene/Entity.h>

namespace Moon {
	LightComponent::LightComponent(Entity& p_owner) :
		AComponent(p_owner),
		m_data{ p_owner.GetComponent<TransformComponent>()->GetTransform() } {
	}

	std::string LightComponent::GetName() {
		return "Light";
	}

	Rendering::Light& LightComponent::GetData() {
		return m_data;
	}

	void LightComponent::SetColor(const glm::vec3& p_color) {
		m_data.color = p_color;
	}

	glm::vec3 LightComponent::GetColor() const {
		return m_data.color;
	}

	void LightComponent::SetStrength(float strength) {
		m_data.strength = strength;
	}
}