#include "AudioSource.h"
#include <Core/Audio/Audio.h>
#include <soloud.h>

Moon::Event<Moon::Audio::AudioSource&> Moon::Audio::AudioSource::CreatedEvent;
Moon::Event<Moon::Audio::AudioSource&> Moon::Audio::AudioSource::DestroyedEvent;

Moon::Audio::AudioSource::AudioSource(AudioEngine& audioEngine, std::optional<std::reference_wrapper<Transform>> p_transform) :
	m_audioEngine(audioEngine), m_transform(p_transform) {
	CreatedEvent.Invoke(*this);
}

Moon::Audio::AudioSource::~AudioSource() {
	Stop();
	DestroyedEvent.Invoke(*this);
}

void Moon::Audio::AudioSource::Update() {

	if (m_spatial) {
		m_audioEngine.GetBackend()->set3dSourceParameters(
			handle,
			m_transform->get().GetWorldPosition().x, m_transform->get().GetWorldPosition().y, m_transform->get().GetWorldPosition().z,
			0.0f, 0.0f, 0.0f
		);
	}
}

void Moon::Audio::AudioSource::Play(const std::string& filepath) {
	Stop();

	if (m_spatial) {
		handle = m_audioEngine.Play3D(
			filepath,
			m_transform->get().GetWorldPosition(),
			m_volume
		);

		constexpr float kMaxDistance = 1000000000.0f;
		//m_audioEngine.GetBackend()->set3dSourceMinMaxDistance(handle, 0.1f, kMaxDistance);
		m_audioEngine.GetBackend()->set3dSourceAttenuation(handle, SoLoud::AudioSource::EXPONENTIAL_DISTANCE, 1.0f);

		m_audioEngine.GetBackend()->update3dAudio();
	}

	if (handle >= 0) {
		m_audioEngine.GetBackend()->setVolume(handle, m_volume);
		m_audioEngine.GetBackend()->setLooping(handle, m_looped);
		m_audioEngine.GetBackend()->setPause(handle, false);
	}
}

void Moon::Audio::AudioSource::Stop() const {
	m_audioEngine.GetBackend()->stop(handle);
	m_audioEngine.GetBackend()->destroyVoiceGroup(handle);
}

bool Moon::Audio::AudioSource::IsValid() const {
	return m_audioEngine.GetBackend()->isValidVoiceHandle(handle);
}

Moon::Transform& Moon::Audio::AudioSource::GetTransform() {
	return m_transform->get();
}
