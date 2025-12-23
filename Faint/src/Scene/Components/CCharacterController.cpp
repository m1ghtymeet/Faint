#include "CCharacterController.h"
#include <Scene/Entity.h>
#include <Physics/Physics.h>
#include "Physics/CharacterController.h"

Moon::CharacterControllerComponent::CharacterControllerComponent(Moon::Entity& p_owner) :
	ColliderComponent(p_owner) {

	m_characterController = std::make_unique<Physics::CharacterController>(p_owner.GetComponent<TransformComponent>()->GetTransform());
	m_characterController->SetUserData<std::reference_wrapper<ColliderComponent>>(*this);
}

Moon::CharacterControllerComponent::~CharacterControllerComponent() {
}

std::string Moon::CharacterControllerComponent::GetName() {
	return "CharacterController";
}

std::unique_ptr<Moon::Physics::CharacterController>& Moon::CharacterControllerComponent::GetCharacterController() {
	return m_characterController;
}

bool Moon::CharacterControllerComponent::IsGrounded() const {
	return m_characterController->IsGrounded();
}

json Moon::CharacterControllerComponent::Serialize() {
	BEGIN_SERIALIZE();
	j["Radius"] = m_characterController->GetSettings().radius;
	j["Height"] = m_characterController->GetSettings().height;
	END_SERIALIZE();
}

void Moon::CharacterControllerComponent::Deserialize(const json& j) {

}
