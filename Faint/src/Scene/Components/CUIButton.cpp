#include "CUIButton.h"

Moon::UIButtonComponent::UIButtonComponent(Entity& p_owner) : AComponent(p_owner)
{
}

std::string Moon::UIButtonComponent::GetName() {
	return "UI Button";
}

json Moon::UIButtonComponent::Serialize() {
	BEGIN_SERIALIZE();
	END_SERIALIZE();
}

void Moon::UIButtonComponent::Deserialize(const json& j) {

}
