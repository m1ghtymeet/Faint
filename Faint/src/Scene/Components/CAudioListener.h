#pragma once

#include "AssetManagment/Serializable.h"
#include <Core/Audio/Audio.h>
#include "AComponent.h"

namespace Moon {
	class AudioListenerComponent : public AComponent {
	public:
		AudioListenerComponent(Entity& p_owner);

		std::string GetName() override;

		json Serialize();

		void Deserialize(const json& j);
	
	private:
		virtual void OnEnable() override;
		virtual void OnDisable() override;

	private:
		Audio::AudioListener m_audioListener;
	};
}