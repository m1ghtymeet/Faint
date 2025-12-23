#pragma warning(push, 0)
//#include "PxPhysicsAPI.h"
#include <geometry/PxGeometryHelpers.h>
#pragma warning(pop)

#include "PhysicsEngine.h"
#include "CollisionReports.h"
#include <algorithm>
#include <iostream>

std::map<std::pair<Moon::Physics::PhysicsActor*, Moon::Physics::PhysicsActor*>, bool> Moon::Physics::PhysicsEngine::m_collisionEvents;
std::map<std::pair<Moon::Physics::PhysicsActor*, Moon::Physics::PhysicsActor*>, bool> Moon::Physics::PhysicsEngine::m_cctContacts;
std::unordered_set<std::pair<Moon::Physics::PhysicsActor*, Moon::Physics::PhysicsActor*>, Moon::Physics::PairHash> Moon::Physics::PhysicsEngine::m_currentFrameContacts;

namespace {
	physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize) {
		PX_UNUSED(attributes0);
		PX_UNUSED(attributes1);
		PX_UNUSED(constantBlockSize);
		PX_UNUSED(constantBlock);
		// generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 == Moon::Physics::CollisionGroup::NO_COLLISION) {
			return physx::PxFilterFlag::eKILL;
		}
		else if ((filterData0.word2 & filterData1.word1) && (filterData1.word2 & filterData0.word1)) {
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eKILL;
	}

	class UserErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual void reportError(physx::PxErrorCode::Enum /*code*/, const char* message, const char* file, int line) {
			std::cout << file << " line " << line << ": " << message << "\n";
			std::cout << "\n";
		}
	}gErrorCallback;

	physx::PxDefaultAllocator g_allocator;
	CCTHitCallback g_cctHitCallback;
}

Moon::Physics::PhysicsEngine::PhysicsEngine() {
	Initialize();

	ListenToPhysicsActors();
}

Moon::Physics::PhysicsEngine::~PhysicsEngine() {
	
}

void Moon::Physics::PhysicsEngine::Update(float p_deltaTime, bool simulate) {
	std::for_each(m_physicsActors.begin(), m_physicsActors.end(), std::mem_fn(&PhysicsActor::UpdatePhysxTransform));
	//for (auto& wrapper : m_physicsActors) {
	//	PhysicsActor& actor = wrapper.get();
	//	if (dynamic_cast<CharacterController*>(&actor) != nullptr)
	//		continue;
	//	actor.UpdatePhysxTransform();
	//}
	//for (auto& wrapper : m_physicsActors)
	//	wrapper.get().UpdateTransform();
	ResetCollisionEvents();

	m_accumulator += p_deltaTime;
	while (m_accumulator >= m_fixedDeltaTime) {
		m_scene->simulate(m_fixedDeltaTime);
		m_scene->fetchResults(true);
		m_accumulator -= m_fixedDeltaTime;
	}

	std::for_each(m_physicsActors.begin(), m_physicsActors.end(), std::mem_fn(&PhysicsActor::UpdateTransform));
	CheckCollisionStopEvents();
}

void Moon::Physics::PhysicsEngine::QueuePhysicsChange(const std::function<void()>& change) {
	std::lock_guard<std::mutex> lock(m_changeMutex);
	m_pendingChanges.push_back(change);
}

std::vector<Moon::Physics::PhysXRayResult> Moon::Physics::PhysicsEngine::CastPhysXRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, uint32_t collisionFlags, bool cullBackFacing) {
	PxVec3 pxOrigin = PxVec3(origin.x, origin.y, origin.z);
	PxVec3 pxDir = PxVec3(direction.x, direction.y, direction.z).getNormalized();

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;//| PxHitFlag::eDISTANCE;
	if (!cullBackFacing)
		hitFlags |= PxHitFlag::eMESH_BOTH_SIDES;

	PxQueryFilterData filterData;
	filterData.data.word0 = collisionFlags;

	constexpr PxU32 MAX_HITS = 64;
	PxRaycastHit hits[MAX_HITS];
	PxRaycastBuffer hitBuffer(hits, MAX_HITS);

	std::vector<PhysXRayResult> results;

	bool status = m_scene->raycast(pxOrigin, pxDir, maxDistance, hitBuffer, hitFlags, filterData);
	if (!status || hitBuffer.getNbAnyHits() == 0)
		return results;	

	const PxU32 hitCount = hitBuffer.getNbAnyHits();
	for (PxU32 i = 0; i < hitCount; i++) {
		const PxRaycastHit& hit = hitBuffer.getAnyHit(i);
		PhysXRayResult result;
		result.found = true;
		result.position = glm::vec3(hit.position.x, hit.position.y, hit.position.z);
		result.normal = glm::vec3(hit.normal.x, hit.normal.y, hit.normal.z);
		result.distance = hit.distance;

		if (hit.actor) {
			result.actor = static_cast<PhysicsActor*>(hit.actor->userData);
			result.name = result.actor ? result.actor->m_name : "";
		}
		else result.name = "NO_HIT";

		results.push_back(result);
	}

	return results;
}

Moon::Physics::PhysXRayResult Moon::Physics::PhysicsEngine::CastPhysXRay(const glm::vec3& p_origin, const glm::vec3& direction, float p_maxDistance, uint32_t collisionFlags, bool cullBackFacing, bool firstOnly) {
	PxVec3 origin = PxVec3(p_origin.x, p_origin.y, p_origin.z);
	PxVec3 unitDir = PxVec3(direction.x, direction.y, direction.z);
	PxReal maxDistance = p_maxDistance;
	PxRaycastBuffer hit;
	PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;// | PxHitFlag::eMESH_BOTH_SIDES;

	if (!cullBackFacing) {
		outputFlags |= PxHitFlag::eMESH_BOTH_SIDES;
	}

	PxQueryFilterData filterData = PxQueryFilterData();
	filterData.data.word0 = collisionFlags; // Or a specific bit for "ray"
	filterData.data.word1 = 0xFFFFFFFF; // Collide with all layers/types defined in your shape's word0
	filterData.data.word2 = 0;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	// Defaults
	PhysXRayResult result;
	result.name = "NO_USERDATA";
	result.position = glm::vec3(0, 0, 0);
	result.normal = glm::vec3(0, 0, 0);
	result.direction = direction;

	//RaycastFilterCallback callback;
	//callback.m_ignoredActors = GetIgnoreList(ignoreFlags);
	//callback.m_ignoredActors.insert(callback.m_ignoredActors.end(), ignoredActors.begin(), ignoredActors.end());

	result.found = m_scene->raycast(origin, unitDir, maxDistance, hit, outputFlags, filterData/*, &callback*/);

	// On hit
	if (result.found) {
		result.position = glm::vec3(hit.block.position.x, hit.block.position.y, hit.block.position.z);
		result.normal = glm::vec3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z);
		result.found = true;
		PhysicsActor* actor = static_cast<PhysicsActor*>(hit.block.actor->userData);
		result.actor = actor;
		result.name = result.actor ? result.actor->m_name : "NO_HIT";
	}
	return result;
}

physx::PxPhysics* Moon::Physics::PhysicsEngine::PxPhysics() const {
	return m_physics;
}

physx::PxScene* Moon::Physics::PhysicsEngine::PxScene() const {
	return m_scene;
}

physx::PxMaterial* Moon::Physics::PhysicsEngine::GetDefaultMaterial() const {
	return m_defaultMaterial;
}

Moon::Physics::PhysicsEngine::MyControllerHitReport* Moon::Physics::PhysicsEngine::GetControllerHitReport() const {
	return m_characterControllerCallback.get();
}

physx::PxControllerManager* Moon::Physics::PhysicsEngine::GetCharacterControllerManager() const {
	return m_characterControllerManager;
}

std::vector<std::reference_wrapper<Moon::Physics::PhysicsActor>> Moon::Physics::PhysicsEngine::GetPhysicsActors() const {
	return m_physicsActors;
}

void Moon::Physics::PhysicsEngine::Initialize() {
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_allocator, gErrorCallback);
	m_pvd = physx::PxCreatePvd(*m_foundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale(), true, m_pvd);
	physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());

	m_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	m_contactCallback = std::make_unique<ContactReportCallback>();
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = m_contactCallback.get();

	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	sceneDesc.contactReportStreamBufferSize = 1024 * 1024;

	m_scene = m_physics->createScene(sceneDesc);
	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);

	physx::PxPvdSceneClient* pvdClient = m_scene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.6f);

	m_characterControllerCallback = std::make_unique<MyControllerHitReport>();
	m_characterControllerManager = PxCreateControllerManager(*m_scene);
}

void Moon::Physics::PhysicsEngine::ListenToPhysicsActors() {

	PhysicsActor::CreatedEvent += std::bind(static_cast<void(PhysicsEngine::*)(PhysicsActor&)>(&PhysicsEngine::Consider), this, std::placeholders::_1);
	PhysicsActor::DestroyedEvent += std::bind(static_cast<void(PhysicsEngine::*)(PhysicsActor&)>(&PhysicsEngine::UnConsider), this, std::placeholders::_1);

	PhysicsActor::ConsiderEvent += std::bind(static_cast<void(PhysicsEngine::*)(physx::PxRigidActor&)>(&PhysicsEngine::Consider), this, std::placeholders::_1);
	PhysicsActor::UnconsiderEvent += std::bind(static_cast<void(PhysicsEngine::*)(physx::PxRigidActor&)>(&PhysicsEngine::UnConsider), this, std::placeholders::_1);
}

void Moon::Physics::PhysicsEngine::Consider(PhysicsActor& p_toConsider) {
	//QueuePhysicsChange([&]() {
		m_physicsActors.push_back(std::ref(p_toConsider));
	//});
}

void Moon::Physics::PhysicsEngine::UnConsider(PhysicsActor& p_toUnconsider) {
	//QueuePhysicsChange([&]() {
		auto found = std::find_if(m_physicsActors.begin(), m_physicsActors.end(), [&p_toUnconsider](std::reference_wrapper<PhysicsActor> element)
			{
				return std::addressof(p_toUnconsider) == std::addressof(element.get());
			});
		if (found != m_physicsActors.end())
			m_physicsActors.erase(found);

		decltype(m_collisionEvents)::iterator iter = m_collisionEvents.begin();
		decltype(m_collisionEvents)::iterator endIter = m_collisionEvents.end();

		for (; iter != endIter; ) {
			if (iter->first.first == std::addressof(p_toUnconsider) || iter->first.second == std::addressof(p_toUnconsider))
				m_collisionEvents.erase(iter++);
			else
				++iter;
		}
	//});
}

void Moon::Physics::PhysicsEngine::Consider(physx::PxRigidActor& actor) {
	if (m_scene)
		m_scene->addActor(actor);
}

void Moon::Physics::PhysicsEngine::UnConsider(physx::PxRigidActor& actor) {
	if (m_scene)
		m_scene->removeActor(actor);
}

void Moon::Physics::PhysicsEngine::ResetCollisionEvents() {
	for (auto& element : m_collisionEvents)
		element.second = false;
}

void Moon::Physics::PhysicsEngine::CheckCollisionStopEvents() {
	for (auto it = m_collisionEvents.begin(); it != m_collisionEvents.end();) {
		auto objects = it->first;
		if (!it->second) {
			if (!objects.first->IsTrigger() && !objects.second->IsTrigger()) {
				objects.first->CollisionStopEvent.Invoke(*objects.second);
				objects.second->CollisionStopEvent.Invoke(*objects.first);
			}
			else {
				//if (objects.first->IsTrigger())
				//	objects.first->TriggerExitEvent.Invoke(*objects.second);
				//else
				//	objects.second->TriggerExitEvent.Invoke(*objects.first);
			}

			it = m_collisionEvents.erase(it);
		}
		else
			++it;
	}

	m_currentFrameContacts.clear();
}

void Moon::Physics::PhysicsEngine::ContactReportCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
	for (PxU32 i = 0; i < count; i++) {
		auto* triggerActor = static_cast<PhysicsActor*>(pairs[i].triggerActor->userData);
		auto* otherActor = static_cast<PhysicsActor*>(pairs[i].otherActor->userData);
		if (!triggerActor || !otherActor) continue;

		auto key = std::make_pair(triggerActor, otherActor);

		if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
			if (m_collisionEvents.find(key) == m_collisionEvents.end()) {
				triggerActor->TriggerEnterEvent.Invoke(*otherActor);
				m_collisionEvents[key] = true;
			}
		}
		else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST) {
			if (m_collisionEvents.find(key) != m_collisionEvents.end()) {
				triggerActor->TriggerExitEvent.Invoke(*otherActor);
				m_collisionEvents.erase(key);
			}
		}
	}
}

void Moon::Physics::PhysicsEngine::ContactReportCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* /*pairs*/, PxU32 /*nbPairs*/) {
    if (!pairHeader.actors[0] || !pairHeader.actors[1]) return;
	auto* actorA = reinterpret_cast<PhysicsActor*>(pairHeader.actors[0]->userData);
	auto* actorB = reinterpret_cast<PhysicsActor*>(pairHeader.actors[1]->userData);

	if (actorA && actorB) {
		if (!actorA->IsTrigger() || !actorB->IsTrigger()) {
			if (m_collisionEvents.find({ actorA, actorB }) == m_collisionEvents.end()) {

				if (actorA->IsTrigger()) {
					actorA->TriggerEnterEvent.Invoke(*actorB);
				}
				else {
					if (!actorB->IsTrigger())
						actorA->CollisionStartEvent.Invoke(*actorB);
				}
				if (actorB->IsTrigger())
					actorB->TriggerEnterEvent.Invoke(*actorA);
				else {
					if (!actorA->IsTrigger())
						actorB->CollisionStartEvent.Invoke(*actorA);
				}

				m_collisionEvents[{ actorA, actorB }] = true;
			}
			else {
				if (!m_collisionEvents[{ actorA, actorB }]) {
					
					m_collisionEvents[{ actorA, actorB }] = true;
				}
			}
		}
	}
}

void Moon::Physics::PhysicsEngine::MyControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& hit) {
	auto* actor = reinterpret_cast<PhysicsActor*>(hit.shape->getActor()->userData);
	auto* controller = reinterpret_cast<PhysicsActor*>(hit.controller->getActor()->userData);

	if (!actor || !controller)
		return;

	auto key = std::make_pair(controller, actor);
	m_currentFrameContacts.insert(key);
	if (m_cctContacts.find(key) == m_cctContacts.end()) {
		if (actor->IsTrigger()) {
			controller->TriggerEnterEvent.Invoke(*actor);
		}
		else {
		}
		m_cctContacts[key] = true;
	}
}

void Moon::Physics::PhysicsEngine::MyControllerHitReport::onControllerHit(const physx::PxControllersHit& hit)
{
}

void Moon::Physics::PhysicsEngine::MyControllerHitReport::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{
}
