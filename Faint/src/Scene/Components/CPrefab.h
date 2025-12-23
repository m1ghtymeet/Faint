#pragma once

#include "AssetManagment/Serializable.h"
#include <Core/Audio/Audio.h>
#include "AComponent.h"

namespace Moon {
	class PrefabComponent : public AComponent {
	public:
		PrefabComponent(Entity& p_owner);

		std::string GetName() override;

		std::string filepath;
		json overridenComponents;
		int id;

		json Serialize();

		void Deserialize(const json& j);

	private:
		Audio::AudioSource* m_audioSource = nullptr;
		bool m_autoPlay = false;
	};
}