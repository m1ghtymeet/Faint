#pragma once
#include "CLight.h"
#include "AssetManagment/Serializable.h"

#include "Renderer/Light.h"

namespace Moon {
	class PointLightComponent : public LightComponent {
	public:
		PointLightComponent(Entity& p_owner);

		void SetRadius(float radius);

		std::string GetName() override;

		json Serialize();

		void Deserialize(const json& j);
	};
}