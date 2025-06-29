#include "CLight.h"

namespace Faint {
	LightComponent::LightComponent(Entity& p_owner) : AComponent(p_owner) {
		Direction = Vec3(0, 1, 0);
		Color = Vec3(1, 1, 1);
		Strength = 1.0f;
		Radius = 16.0f;
		SetCastShadows(true);
	}
	LightComponent::~LightComponent() {}
	void LightComponent::Update(float deltaTime) {
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 2048.0f / 2048.0f, 0.1f, 10.0f);
	}
	void LightComponent::SetCastShadows(bool toggle) {
		CastShadows = toggle;
	}
}