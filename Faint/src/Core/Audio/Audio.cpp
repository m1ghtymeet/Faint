#include "Audio.h"
#include <Debug/Assertion.h>
#include <soloud.h>
#include <soloud_wav.h>
#include <algorithm>
#include <format>
#include <ranges>

Moon::Audio::AudioEngine::AudioEngine() {
	m_soloud = new SoLoud::Soloud();

	if (m_soloud->init() != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR) {
		HZ_CORE_ERROR("Failed to initialize the audio engine. Playback requests will be ignored.");
		delete m_soloud;
		m_soloud = nullptr;
		return;
	}

	using AudioSourceReceiver = void(AudioEngine::*)(AudioSource&);
	using AudioListenerReceiver = void(AudioEngine::*)(AudioListener&);

	AudioSource::CreatedEvent += std::bind(static_cast<AudioSourceReceiver>(&AudioEngine::Consider), this, std::placeholders::_1);
	AudioSource::DestroyedEvent += std::bind(static_cast<AudioSourceReceiver>(&AudioEngine::Unconsider), this, std::placeholders::_1);
	AudioListener::CreatedEvent += std::bind(static_cast<AudioListenerReceiver>(&AudioEngine::Consider), this, std::placeholders::_1);
	AudioListener::DestroyedEvent += std::bind(static_cast<AudioListenerReceiver>(&AudioEngine::Unconsider), this, std::placeholders::_1);
}

Moon::Audio::AudioEngine::~AudioEngine() {
	if (m_soloud) {
		m_soloud->deinit();
		delete m_soloud;
		m_soloud = nullptr;
	}
}

void Moon::Audio::AudioEngine::Update() {
	
	for (auto& source : m_audioSources) {
		source.get().Update();
	}

	const auto listener = FindMainListener();

	if (listener.has_value()) {
		const auto& transform = listener->get().GetTransform();
		const auto& pos = transform.GetWorldPosition();
		const auto forward = glm::normalize(transform.GetWorldRotation() * glm::vec3(0.0f, 0.0f, -1.0f));
		const auto up = glm::normalize(transform.GetWorldRotation() * glm::vec3(0.0f, 1.0f, 0.0f));
		m_soloud->set3dListenerParameters(
			pos.x, pos.y, pos.z,
			forward.x, forward.y, forward.z,
			up.x, up.y, up.z,
			0.0f, 0.0f, 0.0f
		);
	}
	else {
		m_soloud->set3dListenerPosition(0.0f, 0.0f, 0.0f);
		m_soloud->set3dListenerAt(0.0f, 0.0f, -1.0f);
	}

	m_soloud->update3dAudio();
}

int Moon::Audio::AudioEngine::Play3D(const std::string& path, const glm::vec3& position, float volume) {
	SoLoud::Wav* sound = new SoLoud::Wav();
	sound->load(path.c_str());
	return m_soloud->play3d(
		*sound,
		position.x, position.y, position.z,
		0.0f, 0.0f, 0.0f,
		volume
	);;
}

void Moon::Audio::AudioEngine::Stop(int handle) {
	m_soloud->stop(handle);
}

void Moon::Audio::AudioEngine::Consider(AudioSource& source) {
	m_audioSources.push_back(std::ref(source));
}

void Moon::Audio::AudioEngine::Consider(AudioListener& source) {
	m_audioListeners.push_back(std::ref(source));
}

void Moon::Audio::AudioEngine::Unconsider(AudioSource& source) {
	auto found = std::find_if(m_audioSources.begin(), m_audioSources.end(), [&source](std::reference_wrapper<AudioSource> element)
		{
			return std::addressof(source) == std::addressof(element.get());
		});

	if (found != m_audioSources.end())
		m_audioSources.erase(found);
}

void Moon::Audio::AudioEngine::Unconsider(AudioListener& source) {
	auto found = std::find_if(m_audioListeners.begin(), m_audioListeners.end(), [&source](std::reference_wrapper<AudioListener> element)
		{
			return std::addressof(source) == std::addressof(element.get());
		});

	if (found != m_audioListeners.end())
		m_audioListeners.erase(found);
}

std::optional<std::reference_wrapper<Moon::Audio::AudioListener>> Moon::Audio::AudioEngine::FindMainListener(bool p_disabled) const {
	for (auto& listener : m_audioListeners | std::views::reverse) {
		if (/*p_disabled ||*/ listener.get().IsEnabled()) {
			return listener.get();
		}
	}
	return std::nullopt;
}

SoLoud::Soloud* Moon::Audio::AudioEngine::GetBackend() const {
	return m_soloud;
}
