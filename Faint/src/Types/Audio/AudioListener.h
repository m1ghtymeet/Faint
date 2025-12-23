#pragma once

#include <Types/Audio/AudioSource.h>

namespace Moon::Audio {

	/**
	* Represents the ears of your application.
	* You can have multiple ones but only the last created will be considered by the AudioEngine
	*/
	class AudioListener {
	public:
		AudioListener(std::optional<std::reference_wrapper<Transform>> p_transform = std::nullopt);

		~AudioListener();

		Transform& GetTransform();

		void SetEnable(bool p_enable);

		bool IsEnabled() const;

	public:
		static Event<AudioListener&> CreatedEvent;
		static Event<AudioListener&> DestroyedEvent;
	private:
		std::optional<std::reference_wrapper<Transform>> m_transform;
		bool m_enabled = true;
	};
}