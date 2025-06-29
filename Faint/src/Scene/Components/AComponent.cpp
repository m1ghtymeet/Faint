#include "AComponent.h"
#include "Scene/Entity.h"

Faint::AComponent::AComponent(Entity& p_owner) : owner(p_owner) {

}

Faint::AComponent::~AComponent() {
	if (owner.IsActive()) {
		
	}
}
