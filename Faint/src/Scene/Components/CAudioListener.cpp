#include "CAudioListener.h"
#include <Scene/Entity.h>

Moon::AudioListenerComponent::AudioListenerComponent(Entity& p_owner) :
    AComponent(p_owner),
    m_audioListener(owner.transform->GetTransform()) {
}

std::string Moon::AudioListenerComponent::GetName() {
    return "Audio Listener";
}

json Moon::AudioListenerComponent::Serialize() {
    BEGIN_SERIALIZE();
    END_SERIALIZE();
}

void Moon::AudioListenerComponent::Deserialize(const json& j) {
}

void Moon::AudioListenerComponent::OnEnable() {
    m_audioListener.SetEnable(true);
}

void Moon::AudioListenerComponent::OnDisable() {
    m_audioListener.SetEnable(false);
}
