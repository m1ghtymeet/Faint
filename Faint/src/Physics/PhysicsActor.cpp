#include "PhysicsActor.h"
#include <Physics/Physics.h>
#include <Physics/PhysicsEngine.h>
#include <Core/GlobalLocator.h>

Moon::Event<Moon::Physics::PhysicsActor&> Moon::Physics::PhysicsActor::CreatedEvent;
Moon::Event<Moon::Physics::PhysicsActor&> Moon::Physics::PhysicsActor::DestroyedEvent;
Moon::Event<physx::PxRigidActor&> Moon::Physics::PhysicsActor::ConsiderEvent;
Moon::Event<physx::PxRigidActor&> Moon::Physics::PhysicsActor::UnconsiderEvent;

Moon::Physics::PhysicsActor::PhysicsActor() :
	m_transform(new Transform()),
	m_internalTransform(true)
{
	CollisionStartEvent += [this](PhysicsActor& otherPhysicsActor) {
		UpdatePhysxTransform();
	};
}

Moon::Physics::PhysicsActor::PhysicsActor(Transform& p_transform) :
	m_transform(&p_transform),
	m_internalTransform(false)
{
}

Moon::Physics::PhysicsActor::~PhysicsActor() {

	DestroyBody();
	DestroyedEvent.Invoke(*this);

	if (m_internalTransform)
		delete m_transform;
}

void Moon::Physics::PhysicsActor::DestroyActor() {
	if (m_actor) {
		FTSERVICE(PhysicsEngine).PxScene()->removeActor(*m_actor);
		m_actor->release();
		m_actor = nullptr;
	}
}

//void Moon::Physics::PhysicsActor::SetBodyType(BodyType type) {
//	m_bodyType = type;
//}
//
//Moon::Physics::BodyType Moon::Physics::PhysicsActor::GetBodyType() const {
//	return m_bodyType;
//}

void Moon::Physics::PhysicsActor::SetTrigger(bool trigger) {
	m_isTrigger = trigger;
}

bool Moon::Physics::PhysicsActor::IsTrigger() const {
	return m_isTrigger;
}

void Moon::Physics::PhysicsActor::SetKinematic(bool kinematic) {
	m_kinematic = kinematic;
}

bool Moon::Physics::PhysicsActor::IsKinematic() const {
	return m_kinematic;
}

void Moon::Physics::PhysicsActor::SetEnabled(bool enabled) {
	m_enabled = enabled;

	if (!m_enabled)
		Unconsider();
	else
		Consider();
}

bool Moon::Physics::PhysicsActor::IsEnabled() const {
	return m_enabled;
}

void Moon::Physics::PhysicsActor::SetLinearDamping(float damping) {
	m_linearDamping = damping;
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			dynamic->setLinearDamping(m_linearDamping);
		}
	}
}

float Moon::Physics::PhysicsActor::GetLinearDamping() const {
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			return dynamic->getLinearDamping();
		}
	}
	return m_linearDamping;
}

void Moon::Physics::PhysicsActor::SetAngularDamping(float damping) {
	m_angularDamping = damping;
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			dynamic->setAngularDamping(m_angularDamping);
		}
	}
}

float Moon::Physics::PhysicsActor::GetAngularDamping() const {
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			return dynamic->getAngularDamping();
		}
	}
	return m_angularDamping;
}

void Moon::Physics::PhysicsActor::SetGravityScale(float scale) {
	m_gravityScale = scale;
	if (m_actor) {
		m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, scale == 0.0f);
		if (scale != 0.0f) {
			// For dynamic actors, you might need additional handling
		}
	}
}

float Moon::Physics::PhysicsActor::GetGravityScale() const {
	return m_gravityScale;
}

void Moon::Physics::PhysicsActor::SetMass(float mass) {
	if (mass <= 0.0f) {
		//m_kinematic = true;
		m_mass = 0.0f;
		std::cout << "Warning: Mass set to <=0, making kinematic." << std::endl;
	}
	else {
		m_mass = mass;
		//m_kinematic = false;
	}
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			//if (m_mass > 0.0f) {
			//	PxRigidBodyExt::setMassAndUpdateInertia(*dynamic, m_mass);
			//}
		}
	}
}

float Moon::Physics::PhysicsActor::GetMass() const {
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic)
			return dynamic->getMass();
	}
	return m_mass;
}

void Moon::Physics::PhysicsActor::SetLinearLockFlags(uint8_t flags) {
	m_linearLockFlags = flags;
	ApplyConstraints();
}

uint8_t Moon::Physics::PhysicsActor::GetLinearLockFlags() const {
	return m_linearLockFlags;
}

void Moon::Physics::PhysicsActor::AddForce(const glm::vec3& force, bool autoWake) {
	if (m_actor && !IsKinematic()) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic)
			dynamic->addForce(PxVec3(force.x, force.y, force.z), PxForceMode::eFORCE, autoWake);
	}
}

void Moon::Physics::PhysicsActor::ClearForces() {
	if (m_actor) {
		PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
		if (dynamic) {
			dynamic->clearForce(PxForceMode::eFORCE);
			//dynamic->clearTorque(PxForceMode::eFORCE);
		}
	}
}

void Moon::Physics::PhysicsActor::SetLinearVelocity(const glm::vec3& velocity) {
	if (auto* dynamic = m_actor->is<physx::PxRigidDynamic>()) {
		dynamic->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
	}
}

glm::vec3 Moon::Physics::PhysicsActor::GetLinearVelocity() const {
	if (auto* dynamic = m_actor->is<physx::PxRigidDynamic>()) {
		auto vel = dynamic->getLinearVelocity();
		return glm::vec3(vel.x, vel.y, vel.z);
	}
	return glm::vec3(0.0f);
}

void Moon::Physics::PhysicsActor::SetAngularVelocity(const glm::vec3& velocity) {
	if (auto* dynamic = m_actor->is<physx::PxRigidDynamic>()) {
		dynamic->setAngularVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
	}
}

glm::vec3 Moon::Physics::PhysicsActor::GetAngularVelocity() const {
	if (auto* dynamic = m_actor->is<physx::PxRigidDynamic>()) {
		auto vel = dynamic->getAngularVelocity();
		return glm::vec3(vel.x, vel.y, vel.z);
	}
	return glm::vec3(0.0f);
}

void Moon::Physics::PhysicsActor::SetKinematicTarget(const glm::vec3& position, const glm::quat& rotation) {
	auto* dynamic = m_actor->is<physx::PxRigidDynamic>();
	if (!dynamic || !IsKinematic()) return;
	physx::PxTransform target(
		physx::PxVec3(position.x, position.y, position.z),
		physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
	);
	dynamic->setKinematicTarget(target);
}

void Moon::Physics::PhysicsActor::WakeUp() {
	if (!m_actor) return;
	PxRigidDynamic* actor = m_actor->is<PxRigidDynamic>();
	if (actor && actor->isSleeping() == false)
		actor->wakeUp();
}

void Moon::Physics::PhysicsActor::PutToSleep() {
	if (!m_actor) return;
	PxRigidDynamic* actor = m_actor->is<PxRigidDynamic>();
	if (actor && actor->isSleeping() == false)
		actor->putToSleep();
}

bool Moon::Physics::PhysicsActor::IsSleeping() const {
	if (!m_actor) return false;
	PxRigidDynamic* dyn = m_actor->is<PxRigidDynamic>();
	return dyn ? dyn->isSleeping() : false;
}

void Moon::Physics::PhysicsActor::Consider() {
	if (!m_considered) {
		m_considered = true;
		ConsiderEvent.Invoke(*m_actor);
	}
}

void Moon::Physics::PhysicsActor::Unconsider() {
	if (m_considered) {
		m_considered = false;
		UnconsiderEvent.Invoke(*m_actor);
	}
}

physx::PxRigidActor* Moon::Physics::PhysicsActor::GetActor() {
	return m_actor;
}

const physx::PxRigidActor* Moon::Physics::PhysicsActor::GetActor() const {
	return m_actor;
}

void Moon::Physics::PhysicsActor::Init() {
	CreatedEvent.Invoke(*this);
}

void Moon::Physics::PhysicsActor::RecreateBody() {
	CreateActor();
}

void Moon::Physics::PhysicsActor::UpdatePhysicsMaterial() {
	if (!m_material) {
		m_material = FTSERVICE(PhysicsEngine).PxPhysics()->createMaterial(
			m_staticFriction, m_dynamicFriction, m_bounciness
		);
	}
	else {
		// Update existing material
		m_material->setStaticFriction(m_staticFriction);
		m_material->setDynamicFriction(m_dynamicFriction);
		m_material->setRestitution(m_bounciness);
	}
	// Apply material to shape
	if (m_shape && m_material)
		m_shape->setMaterials(&m_material, 1);
}

void Moon::Physics::PhysicsActor::ApplyDamping()
{
}

void Moon::Physics::PhysicsActor::ApplyConstraints() {
	if (!m_actor) return;

	PxRigidDynamic* dynamic = m_actor->is<PxRigidDynamic>();
	if (dynamic) {
		PxRigidDynamicLockFlags lockFlags;

		// Linear constraints
		if (m_linearLockFlags & 0x01) lockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
		if (m_linearLockFlags & 0x02) lockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
		if (m_linearLockFlags & 0x04) lockFlags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;

		// Angular constraints
		if (m_angularLockFlags & 0x01) lockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
		if (m_angularLockFlags & 0x02) lockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
		if (m_angularLockFlags & 0x04) lockFlags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;

		dynamic->setRigidDynamicLockFlags(lockFlags);
	}
}

void Moon::Physics::PhysicsActor::DestroyBody() {
	if (m_actor) {
		physx::PxScene* scene = FTSERVICE(Physics::PhysicsEngine).PxScene();
		if (scene) {
			scene->removeActor(*m_actor);
		}
		m_actor->release();
		m_actor = nullptr;
	}
}

void Moon::Physics::PhysicsActor::SetBounciness(float bounciness) {
	m_bounciness = bounciness;
	UpdatePhysicsMaterial();
}

float Moon::Physics::PhysicsActor::GetBounciness() const {
	return m_bounciness;
}

void Moon::Physics::PhysicsActor::SetStaticFriction(float friction) {
	m_staticFriction = friction;
	UpdatePhysicsMaterial();
}

float Moon::Physics::PhysicsActor::GetStaticFriction() const {
	return m_staticFriction;
}

void Moon::Physics::PhysicsActor::SetDynamicFriction(float friction) {
	m_dynamicFriction = friction;
	UpdatePhysicsMaterial();
}

float Moon::Physics::PhysicsActor::GetDynamicFriction() const {
	return m_dynamicFriction;
}
