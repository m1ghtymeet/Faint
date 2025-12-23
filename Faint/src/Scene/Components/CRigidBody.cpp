#include "CRigidBody.h"
#include <Scene/Entity.h>
#include <Scene/Components/CCharacterController.h>
#include <Physics/Physics.h>
#include <Physics/PhysicsActor.h>

Moon::RigidBodyComponent::RigidBodyComponent(Entity& p_owner) :
    ColliderComponent(p_owner)
{
}

Moon::RigidBodyComponent::~RigidBodyComponent() = default;

std::string Moon::RigidBodyComponent::GetName() {
    return "Rigid Body";
}

void Moon::RigidBodyComponent::SetKinematic(bool kinematic) {
    if (m_physicsActor)
        m_physicsActor->SetKinematic(kinematic);
}

bool Moon::RigidBodyComponent::IsKinematic() const {
    return m_physicsActor ? m_physicsActor->IsKinematic() : false;
}

void Moon::RigidBodyComponent::SetTrigger(bool trigger) {
    if (m_physicsActor)
        m_physicsActor->SetTrigger(trigger);
}

bool Moon::RigidBodyComponent::IsTrigger() const {
    return m_physicsActor ? m_physicsActor->IsTrigger() : false;
}

void Moon::RigidBodyComponent::SetMass(float mass) {
    m_mass = glm::max(mass, 0.0f);
    if (m_physicsActor)
        m_physicsActor->SetMass(mass);
}

float Moon::RigidBodyComponent::GetMass() const {
    return m_physicsActor ? m_physicsActor->GetMass() : 0.0f;
}

void Moon::RigidBodyComponent::SetLinearDrag(float drag) {
    m_linearDrag = glm::clamp(drag, 0.0f, 1.0f);
    if (m_physicsActor)
        m_physicsActor->SetLinearDamping(m_linearDrag);
}

float Moon::RigidBodyComponent::GetLinearDrag() const {
    return m_physicsActor ? m_physicsActor->GetLinearDamping() : m_linearDrag;
}

void Moon::RigidBodyComponent::SetAngularDrag(float drag) {
    m_angularDrag = glm::clamp(drag, 0.0f, 1.0f);
    if (m_physicsActor)
        m_physicsActor->SetAngularDamping(m_angularDrag);
}

float Moon::RigidBodyComponent::GetAngularDrag() const {
    return m_physicsActor ? m_physicsActor->GetAngularDamping() : m_angularDrag;
}

void Moon::RigidBodyComponent::SetGravityScale(float scale) {
    m_gravityScale = glm::max(scale, 0.0f);
    if (m_physicsActor)
        m_physicsActor->SetGravityScale(m_gravityScale);
}

float Moon::RigidBodyComponent::GetGravityScale() const {
    return m_physicsActor ? m_physicsActor->GetGravityScale() : m_gravityScale;
}

void Moon::RigidBodyComponent::SetFreezePosition(const glm::bvec3& freeze) {
    m_freezePosition = freeze;
    ApplyConstraints();
}

glm::bvec3 Moon::RigidBodyComponent::GetFreezePosition() const {
    return m_freezePosition;
}

void Moon::RigidBodyComponent::SetFreezeRotation(const glm::bvec3& freeze) {
    m_freezeRotation = freeze;
    ApplyConstraints();
}

glm::bvec3 Moon::RigidBodyComponent::GetFreezeRotation() const {
    return m_freezeRotation;
}

void Moon::RigidBodyComponent::SetEnableSleep(bool enable) {
    m_enableSleep = enable;
}

bool Moon::RigidBodyComponent::GetEnableSleep() const {
    return m_enableSleep;
}

void Moon::RigidBodyComponent::SetSleepThreshold(float threshold)
{
}

bool Moon::RigidBodyComponent::GetSleepThreshold() const {
    return false;
}

void Moon::RigidBodyComponent::SetBounciness(float bounciness) {
    m_bounciness = glm::clamp(bounciness, 0.0f, 1.0f);
    UpdatePhysicsMaterial();
}

float Moon::RigidBodyComponent::GetBounciness() const {
    return m_bounciness;
}

void Moon::RigidBodyComponent::SetFriction(float friction) {
    m_friction = glm::clamp(friction, 0.0f, 1.0f);
    UpdatePhysicsMaterial();
}

float Moon::RigidBodyComponent::GetFriction() const {
    return m_friction;
}

void Moon::RigidBodyComponent::SetStaticFriction(float friction) {
    m_staticFriction = glm::clamp(friction, 0.0f, 1.0f);
    UpdatePhysicsMaterial();
}

float Moon::RigidBodyComponent::GetStaticFriction() const {
    return m_physicsActor ? m_physicsActor->GetStaticFriction() : m_staticFriction;
}

void Moon::RigidBodyComponent::SetDynamicFriction(float friction) {
    m_dynamicFriction = glm::clamp(friction, 0.0f, 1.0f);
    UpdatePhysicsMaterial();
}

float Moon::RigidBodyComponent::GetDynamicFriction() const {
    return m_physicsActor ? m_physicsActor->GetDynamicFriction() : m_dynamicFriction;
}

void Moon::RigidBodyComponent::AddForce(const glm::vec3& force) {
    if (m_physicsActor && !IsKinematic()) {
        m_physicsActor->AddForce(force);
        if (!m_enableSleep)
            m_physicsActor->WakeUp();
    }
}

void Moon::RigidBodyComponent::AddTorque(const glm::vec3& torque) {
    if (m_physicsActor && !IsKinematic()) {
        //m_physicsActor->AddTorque(torque);
        if (!m_enableSleep)
            m_physicsActor->WakeUp();
    }
}

void Moon::RigidBodyComponent::SetVelocity(const glm::vec3& velocity) {
    //if (m_physicsActor)
    //    m_physicsActor->SetLinearVelocity(velocity);
}

void Moon::RigidBodyComponent::MovePosition(const glm::vec3& position) {
    if (!m_physicsActor) return;
    if (IsKinematic()) {
        m_physicsActor->SetKinematicTarget(position, owner.transform->GetGlobalRotation());
    }
    else {
        glm::vec3 currentPos = owner.transform->GetGlobalPosition();
        glm::vec3 delta = position - currentPos;
        float physicsTimestep = (1.0f / 60.0f);
        glm::vec3 velocity = delta / physicsTimestep;

        //float maxVelocity = 10.0f;
        //float velocityMagnitude = glm::length(velocity);
        //if (velocityMagnitude > maxVelocity)
        //    velocity = glm::normalize(velocity) * maxVelocity;
        m_physicsActor->SetLinearVelocity(velocity);
    }
}

void Moon::RigidBodyComponent::MoveRotation(const glm::quat& rotation) {
    if (!m_physicsActor) return;
    if (IsKinematic()) {
        m_physicsActor->SetKinematicTarget(owner.transform->GetGlobalPosition(), rotation);
    }
    else {
        // For dynamic bodies, calculate angular velocity
        glm::quat currentRot = owner.transform->GetGlobalRotation();
        glm::quat deltaRot = rotation * glm::inverse(currentRot);

        // Convert to axis-angle
        float angle = 2.0f * acos(glm::clamp(deltaRot.w, -1.0f, 1.0f));
        glm::vec3 axis;

        float sinHalfAngle = sqrt(1.0f - deltaRot.w * deltaRot.w);
        if (sinHalfAngle > 0.001f) {
            axis = glm::vec3(deltaRot.x, deltaRot.y, deltaRot.z) / sinHalfAngle;
        }
        else {
            axis = glm::vec3(1, 0, 0);
        }

        float physicsTimestep = 0.016f;
        glm::vec3 angularVelocity = axis * (angle / physicsTimestep);

        m_physicsActor->SetAngularVelocity(angularVelocity);
    }
}

void Moon::RigidBodyComponent::WakeUp() {
    if (m_physicsActor)
        m_physicsActor->WakeUp();
}

void Moon::RigidBodyComponent::PutToSleep() {
    if (m_physicsActor && m_enableSleep)
        m_physicsActor->PutToSleep();
}

bool Moon::RigidBodyComponent::IsSleeping() const {
    return m_physicsActor ? m_physicsActor->IsSleeping() : false;
}

json Moon::RigidBodyComponent::Serialize() {
    BEGIN_SERIALIZE();
    //j["BodyType"] = static_cast<int>(GetBodyType());

    j["IsKinematic"] = IsKinematic();
    j["Trigger"] = IsTrigger();
    j["Mass"] = m_mass;

    j["LinearDrag"] = m_linearDrag;
    j["AngularDrag"] = m_angularDrag;
    j["GravityScale"] = m_gravityScale;

    j["FreezePositionX"] = m_freezePosition.x;
    j["FreezePositionY"] = m_freezePosition.y;
    j["FreezePositionZ"] = m_freezePosition.z;
    j["FreezeRotationX"] = m_freezeRotation.x;
    j["FreezeRotationY"] = m_freezeRotation.y;
    j["FreezeRotationZ"] = m_freezeRotation.z;

    j["EnableSleep"] = m_enableSleep;
    j["SleepThreshold"] = m_sleepThreshold;

    j["Bounciness"] = GetBounciness();
    j["Friction"] = m_friction;
    j["StaticFriction"] = m_staticFriction;
    j["DynamicFriction"] = m_dynamicFriction;

    END_SERIALIZE();
}

void Moon::RigidBodyComponent::Deserialize(const json& j) {
    //if (j.contains("BodyType")) SetBodyType(static_cast<Physics::BodyType>(j["BodyType"]));

    if (j.contains("IsKinematic")) SetKinematic(j["IsKinematic"]);
    if (j.contains("Trigger")) SetTrigger(j["Trigger"]);
    if (j.contains("Mass")) SetMass(j["Mass"]);

    if (j.contains("LinearDrag")) SetLinearDrag(j["LinearDrag"]);
    if (j.contains("AngularDrag")) SetAngularDrag(j["AngularDrag"]);
    if (j.contains("GravityScale")) SetGravityScale(j["GravityScale"]);

    if (j.contains("FreezePositionX") && j.contains("FreezePositionY") && j.contains("FreezePositionZ")) {
        SetFreezePosition({
            j["FreezePositionX"],
            j["FreezePositionY"],
            j["FreezePositionZ"]
            });
    }
    if (j.contains("FreezeRotationX") && j.contains("FreezeRotationY") && j.contains("FreezeRotationZ")) {
        SetFreezeRotation({
            j["FreezeRotationX"],
            j["FreezeRotationY"],
            j["FreezeRotationZ"]
            });
    }

    if (j.contains("EnableSleep")) SetEnableSleep(j["EnableSleep"]);
    if (j.contains("SleepThreshold")) SetSleepThreshold(j["SleepThreshold"]);

    if (j.contains("Bounciness")) SetBounciness(j["Bounciness"]);
    if (j.contains("Friction")) SetFriction(j["Friction"]);
    if (j.contains("StaticFriction")) SetStaticFriction(j["StaticFriction"]);
    if (j.contains("DynamicFriction")) SetDynamicFriction(j["DynamicFriction"]);
}

void Moon::RigidBodyComponent::OnEnable() {
    ApplyConstraints();
    UpdatePhysicsMaterial();

    if (!m_enableSleep)
        m_physicsActor->WakeUp();
}

void Moon::RigidBodyComponent::OnDisable() {
    // Clean up when component is disabled
}

void Moon::RigidBodyComponent::BindListener() {
    
    /* Collision Events */
    m_physicsActor->CollisionStartEvent += [this](Moon::Physics::PhysicsActor& otherPhysicsObject) {
        auto& otherObject = otherPhysicsObject.GetUserData<std::reference_wrapper<Moon::ColliderComponent>>().get();
        CollisionEnterEvent.Invoke(otherObject);
        owner.OnCollisionEnter(otherObject);
    };
    m_physicsActor->CollisionStopEvent += [this](Moon::Physics::PhysicsActor& otherPhysicsObject) {
        auto& otherObject = otherPhysicsObject.GetUserData<std::reference_wrapper<Moon::ColliderComponent>>().get();
        CollisionExitEvent.Invoke(otherObject);
        owner.OnCollisionExit(otherObject);
    };
    /* Trigger Events */
    m_physicsActor->TriggerEnterEvent += [this](Moon::Physics::PhysicsActor& otherPhysicsObject) {
        auto& otherObject = otherPhysicsObject.GetUserData<std::reference_wrapper<Moon::ColliderComponent>>().get();
        TriggerEnterEvent.Invoke(otherObject);
        owner.OnTriggerEnter(otherObject);
    };
    m_physicsActor->TriggerStayEvent += [this](Moon::Physics::PhysicsActor& otherPhysicsObject) {
        auto& otherObject = otherPhysicsObject.GetUserData<std::reference_wrapper<Moon::ColliderComponent>>().get();
        TriggerStayEvent.Invoke(otherObject);
        owner.OnTriggerStay(otherObject);
        };
    m_physicsActor->TriggerExitEvent += [this](Moon::Physics::PhysicsActor& otherPhysicsObject) {
        auto& otherObject = otherPhysicsObject.GetUserData<std::reference_wrapper<Moon::ColliderComponent>>().get();
        TriggerExitEvent.Invoke(otherObject);
        owner.OnTriggerExit(otherObject);
    };

    /* Sleep/Wake Events */
    // m_physicsActor->SleepEvent += [this]() { SleepEvent.Invoke(); };
    // m_physicsActor->WakeEvent += [this]() { WakeEvent.Invoke(); };
}

void Moon::RigidBodyComponent::ApplyConstraints() {
    if (!m_physicsActor) return;

    // Convert glm::bvec3 to PhysX lock flags
    uint8_t linearFlags = 0;
    uint8_t angularFlags = 0;

    // Apply position constraints
    if (m_freezePosition.x) linearFlags |= 0x01;
    if (m_freezePosition.y) linearFlags |= 0x02;
    if (m_freezePosition.z) linearFlags |= 0x04;

    // Apply rotation constraints
    if (m_freezeRotation.x) angularFlags |= 0x01;
    if (m_freezeRotation.y) angularFlags |= 0x02;
    if (m_freezeRotation.z) angularFlags |= 0x04;

    m_physicsActor->SetLinearLockFlags(linearFlags);
    //m_physicsActor->SetAngularLockFlags(linearFlags);
}

void Moon::RigidBodyComponent::UpdatePhysicsMaterial() {
    if (!m_physicsActor) return;
    // Update physics material properties
    m_physicsActor->SetBounciness(m_bounciness);
    m_physicsActor->SetStaticFriction(m_staticFriction);
    m_physicsActor->SetDynamicFriction(m_dynamicFriction);
}
