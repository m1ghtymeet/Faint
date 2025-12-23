#pragma once
#include "AComponent.h"
#include "AssetManagment/Serializable.h"

#include "Renderer/Light.h"

namespace Moon {
	class LightComponent : public AComponent {
	public:
		LightComponent(Entity& p_owner);

		virtual std::string GetName() override;

		Rendering::Light& GetData();

		void SetColor(const glm::vec3& p_color);
		glm::vec3 GetColor() const;

		void SetStrength(float p_strength);

	protected:
		Rendering::Light m_data;
	};
}