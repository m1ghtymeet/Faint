#include "RigidBodyComponent.h"

namespace Faint {

    RigidBodyComponent::RigidBodyComponent(Entity& p_owner) : AComponent(p_owner), mass(1.0f) {
    
    }
}
