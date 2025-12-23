#include "CharacterController.h"
#include <Debug/Log.h>
#include <Physics/PhysicsEngine.h>
#include <Physics/Physics.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include <Core/GlobalLocator.h>

Moon::Physics::CharacterController::CharacterController(Transform& transform, CharacterControllerSettings settings)
	: PhysicsActor(transform) {
	m_settings = settings;
	if (!m_settings.material)
		m_settings.material = FTSERVICE(PhysicsEngine).GetDefaultMaterial();

	CreateController();
}

Moon::Physics::CharacterController::~CharacterController() {
	if (m_controller) {
		m_controller->release();
		m_controller = nullptr;
	}
}

void Moon::Physics::CharacterController::Move(const glm::vec3& displacement) {
	if (!m_controller) return;

	m_wasGroundedLastFrame = m_isGrounded;

	//if (!glm::epsilonEqual(m_currentHeight, m_targetHeight, 0.001f)) {
	//	m_currentHeight = glm::lerp(m_currentHeight, m_targetHeight, 10.0f);
	//	m_controller->resize(m_currentHeight);
	//}

	PxControllerFilters filters;
	PxFilterData filterData;
	filterData.word0 = CollisionGroup::CHARACTER_CONTROLLER;
	filters.mFilterData = &filterData;
	PxF32 minDist = 0.001f;
	m_lastMoveFlags =
		m_controller->move(Physics::GlmVec3toPxVec3(displacement), minDist, (1.0f / 60.0f), filters);
}

bool Moon::Physics::CharacterController::IsGrounded() const {
	return m_isGrounded;
}

void Moon::Physics::CharacterController::SetPosition(glm::vec3 position) {
	m_controller->setPosition({ position.x, position.y, position.z });
}

glm::vec3 Moon::Physics::CharacterController::GetPosition() const {
	return Physics::PxVec3toGlmVec3(m_controller->getPosition());
}

void Moon::Physics::CharacterController::SetFootPosition(glm::vec3 position) {
	m_controller->setFootPosition({ position.x, position.y, position.z });
}

glm::vec3 Moon::Physics::CharacterController::GetFootPosition() const {
	PxExtendedVec3 feetPos = m_controller->getFootPosition();
	return { feetPos.x, feetPos.y, feetPos.z };
}

physx::PxController* Moon::Physics::CharacterController::GetController() const {
	return m_controller;
}

void Moon::Physics::CharacterController::Recreate() {
	if (!m_controller) {
		Create(m_settings);
		return;
	}

	PxExtendedVec3 prevPos = m_controller->getPosition();
	PxControllerFilters filters;
	m_controller->move(PxVec3(0.0f), 0.0f, 0.0f, filters);

	PxExtendedVec3 prevTransform = m_controller->getPosition();

	m_controller->release();
	m_controller = nullptr;

	CreateController();

	if (m_controller) {
		m_controller->setPosition(prevPos);
		//m_controller->getActor()->setGlobalPose(prevTransform);
	}
}

void Moon::Physics::CharacterController::CreateController() {
	PxCapsuleControllerDesc desc;
	desc.setToDefault();

	desc.height           = m_settings.height;
	desc.radius           = m_settings.radius;
	desc.stepOffset       = m_settings.stepOffset;
	desc.contactOffset    = m_settings.contactOffset;
	desc.density          = 10.0f;
	desc.scaleCoeff       = m_settings.scaleCoeff;
	desc.slopeLimit       = cosf(glm::radians(m_settings.slopeLimitDeg));
	desc.material         = FTSERVICE(PhysicsEngine).GetDefaultMaterial();
	desc.upDirection      = PxVec3(0, 1, 0);
	desc.reportCallback   = FTSERVICE(PhysicsEngine).GetControllerHitReport();
	desc.behaviorCallback = nullptr;

	//const auto& pos = m_transform->GetLocalPosition();
	//desc.position = PxExtendedVec3(pos.x, pos.y, pos.z);

	if (!desc.isValid()) {
		HZ_CORE_ERROR("PxCapsuleControllerDesc is not valid!");
		HZ_CORE_ERROR("height={}, radius={}, stepOffset={}, contactOffset={}",
			desc.height, desc.radius, desc.stepOffset, desc.contactOffset);
		return;
	}

	m_controller = FTSERVICE(PhysicsEngine).GetCharacterControllerManager()->createController(desc);
	if (!m_controller) {
		// Failed to create PxCapsuleController!
		return;
	}
	
	m_controller->getActor()->getShapes(&m_shape, 1);
	PxFilterData filterData;
	filterData.word0 = CollisionGroup::CHARACTER_CONTROLLER;
	filterData.word1 = CollisionGroup(STATIC_OBJECT | DYNAMIC_OBJECT);
	m_shape->setSimulationFilterData(filterData);

	PxFilterData queryFilter;
	queryFilter.word0 = RaycastGroup::RAYCAST_DISABLED;
	m_shape->setQueryFilterData(queryFilter);

	m_controller->getActor()->userData = this;

	m_currentHeight = m_settings.height;
	m_targetHeight = m_settings.height;

	PhysicsActor::Init();
}

void Moon::Physics::CharacterController::UpdateGroundState() {
	const float groundCheckDistance = m_settings.skinWidth + 0.05f;
	const PxVec3 down(0, -groundCheckDistance, 0);

	PxControllerFilters filters;
	PxQueryFilterData qfd;
	qfd.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;

	PxControllerState state;
	m_controller->getState(state);

	m_isGrounded = (m_lastMoveFlags & PxControllerCollisionFlag::eCOLLISION_DOWN);
		//|| state.touchedObstacleHandle;
}

void Moon::Physics::CharacterController::UpdateTransform() {
	if (!m_controller || !m_transform) return;
	PxExtendedVec3 pos = m_controller->getFootPosition();//m_controller->getPosition();
	m_transform->SetLocalPosition({ pos.x, pos.y, pos.z });
	UpdateGroundState();
}

void Moon::Physics::CharacterController::Create(const CharacterControllerSettings& settings) {
	CreateController();
}