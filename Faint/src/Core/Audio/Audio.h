#pragma once

#include <vector>
#include <Types/Audio/AudioSource.h>
#include <Types/Audio/AudioListener.h>

namespace SoLoud {
	class Soloud;
}

namespace Moon::Audio {

	/**
	* AudioEngine is the main class of the audio system.
	* It's responsible for initializing the audio backend and managing the audio sources and listeners.
	*/
	class AudioEngine {
	public:
		AudioEngine();

		~AudioEngine();

		void Update();

		int Play3D(const std::string& path, const glm::vec3& position, float volume = 1.0f);

		void Stop(int handle);
		
		void Consider(AudioSource& source);
		void Consider(AudioListener& source);

		void Unconsider(AudioSource& source);
		void Unconsider(AudioListener& source);

		std::optional<std::reference_wrapper<AudioListener>> FindMainListener(bool p_disabled = false) const;

		SoLoud::Soloud* GetBackend() const;

	private:
		SoLoud::Soloud* m_soloud = nullptr;
		std::vector<std::reference_wrapper<AudioSource>> m_audioSources;
		std::vector<std::reference_wrapper<AudioListener>> m_audioListeners;
	};
}