#include "CWidget.h"

Moon::UI::WidgetComponent::WidgetComponent(Entity& p_owner) :
    AComponent(p_owner) {
}

std::string Moon::UI::WidgetComponent::GetName() {
    return "Widget";
}

void Moon::UI::WidgetComponent::SetTexture(Assets::Texture* texture) {
    m_texture = texture;
}

Moon::Assets::Texture* Moon::UI::WidgetComponent::GetTexture() const {
    return m_texture;
}

void Moon::UI::WidgetComponent::SetColor(const glm::vec3& color) {
    m_color = color;
}

const glm::vec3& Moon::UI::WidgetComponent::GetColor() const {
    return m_color;
}

json Moon::UI::WidgetComponent::Serialize() {
    BEGIN_SERIALIZE();
    END_SERIALIZE();
}

void Moon::UI::WidgetComponent::Deserialize(const json& j) {

}
