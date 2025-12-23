#include "AComponent.h"
#include "Scene/Entity.h"

Moon::AComponent::AComponent(Entity& p_owner) : owner(p_owner) {

}

Moon::AComponent::~AComponent() {
	if (owner.IsActive()) {
		OnDisable();
		OnDestroy();
	}
}
