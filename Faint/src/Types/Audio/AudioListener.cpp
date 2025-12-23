#include "AudioListener.h"

Moon::Event<Moon::Audio::AudioListener&> Moon::Audio::AudioListener::CreatedEvent;
Moon::Event<Moon::Audio::AudioListener&> Moon::Audio::AudioListener::DestroyedEvent;

Moon::Audio::AudioListener::AudioListener(std::optional<std::reference_wrapper<Transform>> p_transform)
	: m_transform(p_transform) {
	CreatedEvent.Invoke(*this);
}

Moon::Audio::AudioListener::~AudioListener() {
	DestroyedEvent.Invoke(*this);
}

Moon::Transform& Moon::Audio::AudioListener::GetTransform() {
	return m_transform.value();
}

void Moon::Audio::AudioListener::SetEnable(bool p_enable) {
	m_enabled = p_enable;
}

bool Moon::Audio::AudioListener::IsEnabled() const {
	return m_enabled;
}
