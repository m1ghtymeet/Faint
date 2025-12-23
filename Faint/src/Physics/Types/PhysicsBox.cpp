#include "PhysicsBox.h"
#include <Core/GlobalLocator.h>
#include <Math/Math.h>
#include <Physics/PhysicsEngine.h>
#include <Physics/Physics.h>

Moon::Physics::PhysicsBox::PhysicsBox(Transform& transform, const glm::vec3& halfExtents, float density) :
	PhysicsActor(transform), m_halfExtents(halfExtents), m_density(density) {
	float volume = 8.0f * halfExtents.x * halfExtents.y * halfExtents.z;
	SetMass(density * volume);
}

void Moon::Physics::PhysicsBox::CreateActor() {

	PxPhysics* physics = FTSERVICE(PhysicsEngine).PxPhysics();
	PxRigidDynamic* dyn = nullptr;

	PxTransform pxTransform(PxVec3(
		m_transform->GetWorldPosition().x,
		m_transform->GetWorldPosition().y,
		m_transform->GetWorldPosition().z
	));

	dyn = physics->createRigidDynamic(pxTransform);
	if (IsKinematic()) {
		dyn->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, false);
		dyn->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD_FRICTION, false);
	}
	dyn->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, m_gravityScale == 0.0f);
	dyn->setAngularDamping(0.0f);
	dyn->setLinearDamping(0.05f);
	dyn->setSleepThreshold(0.0f);
	dyn->setStabilizationThreshold(0.001f);
	dyn->setSolverIterationCounts(8, 4);
	dyn->setMaxDepenetrationVelocity(2.0f);
	m_actor = dyn;
	m_actor->userData = this;
	
	ApplyConstraints();
	UpdatePhysicsMaterial();
	CreateShape();

	if (!IsKinematic() && m_mass > 0.0f)
		PxRigidBodyExt::setMassAndUpdateInertia(*dyn, m_mass);

	PhysicsActor::Init();
}

void Moon::Physics::PhysicsBox::Reshape(const glm::vec3& halfExtents, const glm::vec3& offsetPos, const glm::quat& offsetRot) {
	if (!m_actor) return;

	if (m_shape) {
		m_actor->detachShape(*m_shape);
		m_shape->release();
		m_shape = nullptr;
	}

	m_halfExtents = halfExtents;
	m_shapeOffset = offsetPos;
	float volume = 8.0f * halfExtents.x * halfExtents.y * halfExtents.z;

	auto* physics = FTSERVICE(PhysicsEngine).PxPhysics();

	//PxBoxGeometry boxGeom(PxVec3(halfExtents.x > 0 ? halfExtents.x : 0.1f, halfExtents.y > 0 ? halfExtents.y : 0.1f, halfExtents.z > 0 ? halfExtents.z : 0.1f));
	PxBoxGeometry boxGeom(PxVec3(
		std::max(halfExtents.x, 0.1f),
		std::max(halfExtents.y, 0.1f),
		std::max(halfExtents.z, 0.1f)
	));

	m_shape = physics->createShape(boxGeom, *FTSERVICE(PhysicsEngine).GetDefaultMaterial(), true);
	
	glm::quat normalizedRot = glm::normalize(offsetRot);
	PxTransform localPose(
		PxVec3(offsetPos.x, offsetPos.y, offsetPos.z),
		PxQuat(normalizedRot.x, normalizedRot.y, normalizedRot.z, normalizedRot.w)
	);
	if (!localPose.isValid()) {
		localPose = PxTransform(PxVec3(offsetPos.x, offsetPos.y, offsetPos.z));
	}
	m_shape->setLocalPose(localPose);

	PxFilterData filter;
	filter.word0 = RAYCAST_ENABLED;
	filter.word1 = DYNAMIC_OBJECT;
	filter.word2 = DYNAMIC_OBJECT | STATIC_OBJECT | CHARACTER_CONTROLLER;
	m_shape->setQueryFilterData(filter);
	m_shape->setSimulationFilterData(filter);

	m_actor->attachShape(*m_shape);

	if (auto* dynamic = m_actor->is<PxRigidDynamic>()) {
		PxVec3 centerOfMass = -PxVec3(offsetPos.x, offsetPos.y, offsetPos.z);
		dynamic->setCMassLocalPose(PxTransform(centerOfMass));
		PxRigidBodyExt::updateMassAndInertia(*dynamic, m_density);
	}
}

void Moon::Physics::PhysicsBox::SetShapeOffset(const glm::vec3& offsetPos, const glm::quat& offsetRot) {
	if (!m_shape) return;
	PxTransform localPose(
		PxVec3(offsetPos.x, offsetPos.y, offsetPos.z),
		PxQuat(offsetRot.x, offsetRot.y, offsetRot.z, offsetRot.w)
	);
	if (!localPose.isValid()) {
		localPose = PxTransform(PxVec3(offsetPos.x, offsetPos.y, offsetPos.z));
	}
	m_shape->setLocalPose(localPose);
	if (auto* dynamic = m_actor->is<PxRigidDynamic>()) {
		PxVec3 centerOfMass = -PxVec3(offsetPos.x, offsetPos.y, offsetPos.z);
		dynamic->setCMassLocalPose(PxTransform(centerOfMass));
		PxRigidBodyExt::updateMassAndInertia(*dynamic, m_density);
	}
}

void Moon::Physics::PhysicsBox::CreateShape() {
	auto* physics = FTSERVICE(PhysicsEngine).PxPhysics();
	PxBoxGeometry boxGeom(PxVec3(m_halfExtents.x, m_halfExtents.y, m_halfExtents.z));
	m_shape = physics->createShape(boxGeom, *m_material, true);
	m_shape->setContactOffset(0.05f);
	m_shape->setRestOffset(0.01f);
	if (m_shapeOffset != glm::vec3(0.0f)) {
		PxTransform localPose(PxVec3(m_shapeOffset.x, m_shapeOffset.y, m_shapeOffset.z));
		m_shape->setLocalPose(localPose);
	}
	PxFilterData filter;
	filter.word0 = RAYCAST_ENABLED;
	filter.word1 = DYNAMIC_OBJECT;
	filter.word2 = DYNAMIC_OBJECT | STATIC_OBJECT | CHARACTER_CONTROLLER;
	m_shape->setQueryFilterData(filter);
	m_shape->setSimulationFilterData(filter);
	m_actor->attachShape(*m_shape);
}

void Moon::Physics::PhysicsBox::UpdatePhysxTransform() {
	if (!m_actor || !m_transform) return;
	PxTransform pxT(
		Physics::GlmVec3toPxVec3(m_transform->GetWorldPosition()),
		Physics::GlmQuatToPxQuat(Math::EulerDegreesToQuatSafe(m_transform->GetRotationEuler()))
	);
	PxRigidDynamic* dynamic = m_actor->is<physx::PxRigidDynamic>();
	// Kinematic
	dynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, IsKinematic());
	// Trigger
	if (m_shape) {
		if (IsTrigger()) {
			m_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			m_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}
	}
	if (IsKinematic()) {
		dynamic->setKinematicTarget(pxT);
	}
	else
		dynamic->setGlobalPose(pxT);
	if (dynamic->isSleeping())
		dynamic->wakeUp();
}

void Moon::Physics::PhysicsBox::UpdateTransform() {
	if (!m_actor || !m_transform) return;
	if (!IsKinematic()) {
		PxTransform pxT = m_actor->getGlobalPose();
		m_transform->SetWorldPosition(Physics::PxVec3toGlmVec3(pxT.p));
		m_transform->SetLocalRotation(Physics::PxQuatToGlmQuat(pxT.q));
	}
}