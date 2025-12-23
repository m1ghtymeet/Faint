#include "CAudio.h"
#include <Scene/Entity.h>
#include <Core/GlobalLocator.h>
#include <Core/Audio/Audio.h>
#include <FileSystem/FileSystem.h>

Moon::AudioComponent::AudioComponent(Entity& p_owner) : AComponent(p_owner) {
    m_audioSource = new Audio::AudioSource(GlobalLocator::Get<Audio::AudioEngine>(), owner.transform->GetTransform());
}

std::string Moon::AudioComponent::GetName() {
    return "Audio";
}

void Moon::AudioComponent::Play() {
    m_audioSource->Play(FileSystem::Root + filepath);
}

void Moon::AudioComponent::Stop() const {
    m_audioSource->Stop();
}

void Moon::AudioComponent::SetVolume(float volume) {
    m_audioSource->m_volume = volume;
}

void Moon::AudioComponent::SetSpatial(bool spatial) {
    m_audioSource->m_spatial = spatial;
}

void Moon::AudioComponent::SetAutoplay(bool p_autoplay) {
    m_autoPlay = p_autoplay;
}

bool Moon::AudioComponent::IsSpatial() const {
    return m_audioSource->m_spatial;
}

bool Moon::AudioComponent::IsAutoplayed() const {
    return m_autoPlay;
}

json Moon::AudioComponent::Serialize() {
    BEGIN_SERIALIZE();
    j["filepath"] = filepath;
    j["volume"] = m_audioSource->m_volume;
    END_SERIALIZE();
}

void Moon::AudioComponent::Deserialize(const json& j) {
    filepath = j["filepath"];
    SetVolume(j["volume"]);
}

void Moon::AudioComponent::OnEnable() {
    if (m_autoPlay)
        Play();
}

void Moon::AudioComponent::OnDisable() {
    m_audioSource->Stop();
}
