#pragma once

#include "Core/Base.h"
#include "Math/Math.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace SoLoud
{
	class Soloud;
	class AudioSource;
	class Wav;
}

namespace Faint {

	struct AudioRequest
	{
		std::string audioFile;
		float volume = 1.0f;
		float pan = 0.0f;
		float speed = 1.0f;
		bool spatialized = false;
		Vec3 position;
		float MinDistance = 1.0f;
		float MaxDistance = 25.0f;
		float AttenuationFactor = 1.0f;
		bool Loop = false;
	};

	class AudioManager {
	private:
		int m_MaxVoiceCount = 32;
		float m_GlobalVolume = 1.0f;

		Ref<SoLoud::Soloud> m_Soloud;

		bool m_AudioQueued = false;
		std::queue<AudioRequest> m_AudioQueue;

		Vec3 m_ListenerPosition;
		Vec3 m_ListenerDirection;
		Vec3 m_ListenerUp;

		std::unordered_map<std::string, Ref<SoLoud::AudioSource>> m_WavSamples;
		std::unordered_map<std::string, unsigned int> m_ActiveClips;

	public:
		AudioManager();
		~AudioManager();

		static AudioManager& Get()
		{
			static AudioManager instance;
			return instance;
		}

		void Initialize();
		void Deinitialize();

		void SetListenerPosition(const Vec3& position, const Vec3& direction, const Vec3& up);
		float GetVolume() const;
		void SetVolume(float volume);

		void QueueWavAudio(const AudioRequest& request);
		void UpdateVoice(const AudioRequest& request);

		void StopVoice(const std::string& filePath);
		void StopAll() const;

		bool IsWavLoaded(const std::string& filePath) const;
		bool IsVoiceActive(const std::string& voice) const;
		void LoadWavAudio(const std::string& filePath);

		void AudioUpdate();
	};
}