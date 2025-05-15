#include "RigidBodyComponent.h"

namespace Faint {

    RigidBodyComponent::RigidBodyComponent() : mass(0.0f) {
    
    }

    PxRigidDynamic* RigidBodyComponent::GetRigidBody() const {
        return dynamicActor;
    }

    //Ref<RigidDynamic> RigidBodyComponent::GetRigidBody() const {
    //    return Rigidbody;
    //}
}
