#include "AudioSystem.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"

#include "Core/Audio.h"

namespace Faint {

	AudioSystem::AudioSystem(Scene* scene)
	{
		m_scene = scene;
	}

	bool AudioSystem::Init()
	{
		AudioManager::Get().StopAll();

		return true;
	}

	void AudioSystem::Update(Time ts)
	{
		auto& audioManager = AudioManager::Get();

		auto currentCamera = m_scene->GetCurrentCamera();

		Vec3 direction = currentCamera->GetForward();
		if (Engine::IsPlayMode())
			direction *= -1.0f;

		audioManager.SetListenerPosition(currentCamera->GetPosition(), std::move(direction), currentCamera->GetUp());
	
		auto view = m_scene->Reg().view<TransformComponent, AudioComponent>();
		for (auto& e : view)
		{
			auto [transform, audioEmitter] = view.get<TransformComponent, AudioComponent>(e);

			if (audioEmitter.FilePath.empty())
				continue;

			const bool isPlaying = audioEmitter.IsPlaying;
			const std::string absoluteFilePath = FileSystem::RelativeToAbsolute(audioEmitter.FilePath);
			const bool isVoiceActive = audioManager.IsVoiceActive(absoluteFilePath);
		
			AudioRequest audioRequest;
			audioRequest.audioFile = absoluteFilePath;
			audioRequest.pan = audioEmitter.Pan;
			audioRequest.volume = audioEmitter.Volume;
			audioRequest.speed = audioEmitter.PlaybackSpeed;
			audioRequest.spatialized = audioEmitter.Spatialized;
			audioRequest.Loop = audioEmitter.Loop;
			audioRequest.position = transform.GetGlobalPosition();
			audioRequest.MinDistance = audioEmitter.MinDistance;
			audioRequest.MaxDistance = audioEmitter.MaxDistance;
			audioRequest.AttenuationFactor = audioEmitter.AttenuationFactor;

			if (isVoiceActive)
			{
				if (!isPlaying && audioEmitter.Loop)
				{
					audioManager.StopVoice(absoluteFilePath); // Stop audio
				}
				else
				{
					// Update the active voice with new params
					audioManager.UpdateVoice(audioRequest);
				}
			}

			if (isPlaying)
			{
				// Reset the play status to false since the audio has been fired
				if (!audioEmitter.Loop)
				{
					audioManager.QueueWavAudio(std::move(audioRequest));
					audioEmitter.IsPlaying = false;
				}
				else if (!isVoiceActive)
				{
					audioManager.QueueWavAudio(std::move(audioRequest));
				}
			}
		}
	}

	void AudioSystem::FixedUpdate(Time ts)
	{
	}

	void AudioSystem::Exit()
	{
		AudioManager::Get().StopAll();
	}
}
