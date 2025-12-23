#pragma once
#include "CLight.h"
#include "AssetManagment/Serializable.h"

#include "Renderer/Light.h"

namespace Moon {
	class DirectionalLightComp : public LightComponent {
	public:
		DirectionalLightComp(Entity& p_owner);

		std::string GetName() override;

		json Serialize();

		void Deserialize(const json& j);
	};
}