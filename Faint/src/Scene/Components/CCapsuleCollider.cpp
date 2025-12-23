#include "CCapsuleCollider.h"

Moon::CapsuleColliderComponent::CapsuleColliderComponent(Entity& p_owner) :
	AComponent(p_owner) {
}

json Moon::CapsuleColliderComponent::Serialize() {
	BEGIN_SERIALIZE();
	SERIALIZE_VAL(radius);
	SERIALIZE_VAL(height);
	END_SERIALIZE();
}

void Moon::CapsuleColliderComponent::Deserialize(const json& j) {
	this->radius = j["radius"];
	this->height = j["height"];
}
