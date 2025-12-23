#pragma once

#include "AssetManagment/Serializable.h"
#include <Core/Audio/Audio.h>
#include "AComponent.h"

namespace Moon {
	class AudioComponent : public AComponent {
	public:
		AudioComponent(Entity& p_owner);

		std::string GetName() override;

		void Play();

		void Stop() const;

		void SetVolume(float volume);

		void SetSpatial(bool spatial);

		void SetAutoplay(bool p_autoplay);

		bool IsSpatial() const;

		bool IsAutoplayed() const;

		std::string filepath;

		json Serialize();

		void Deserialize(const json& j);

	private:
		virtual void OnEnable() override;
		virtual void OnDisable() override;
	private:
		Audio::AudioSource* m_audioSource = nullptr;
		bool m_autoPlay = false;
	};
}