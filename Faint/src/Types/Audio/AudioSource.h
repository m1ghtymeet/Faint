#pragma once

#include <Event/Event.h>
#include <Math/Transform.h>
#include <string>
#include <optional>

namespace Moon::Audio {

	class AudioEngine;

	/**
	* An audio source is an entity that can play a sound in a 3D space.
	*/
	class AudioSource {
	public:
		AudioSource(
			AudioEngine& audioEngine,
			std::optional<std::reference_wrapper<Transform>> p_transform = std::nullopt
		);

		~AudioSource();

		void Update();

		void Play(const std::string& filepath);

		void Stop() const;

		bool IsValid() const;

		Transform& GetTransform();

	public:
		static Event<AudioSource&> CreatedEvent;
		static Event<AudioSource&> DestroyedEvent;

	public:
		bool m_spatial = true;
		float m_volume = 1.0f;
		float m_pan = 0.0f;
		bool m_looped = false;
		int handle = -1;

	private:
		AudioEngine& m_audioEngine;
		std::optional<std::reference_wrapper<Transform>> m_transform;
	};
}