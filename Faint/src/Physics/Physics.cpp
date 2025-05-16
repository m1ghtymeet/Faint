#include "Physics.h"
#include <iostream>
#include <entt.hpp>

PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	if (filterData0.word2 == CollisionGroup::NO_COLLISION) {
		return PxFilterFlag::eKILL;
	}
	else if ((filterData0.word2 & filterData1.word1) && (filterData1.word2 & filterData0.word1)) {
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		return PxFilterFlag::eDEFAULT;
	}

	return PxFilterFlag::eKILL;
}

namespace Faint::Physics {

	class UserErrorCallback : public PxErrorCallback
	{
	public:
		virtual void reportError(PxErrorCode::Enum /*code*/, const char* message, const char* file, int line) {
			std::cout << file << " line " << line << ": " << message << "\n";
			std::cout << "\n";
		}
	}gErrorCallback;

	PxPhysics* g_physics = NULL;
	PxScene* g_scene = NULL;
	PxFoundation* g_foundation;
	PxDefaultAllocator g_allocator;
	PxDefaultCpuDispatcher* g_dispatcher = NULL;
	PxPvd* g_pvd = NULL;
	PxMaterial* g_defaultMaterial = NULL;
	PxMaterial* g_grassMaterial = NULL;
	PxControllerManager* g_characterControllerManager;
	std::vector<CollisionReport> g_collisionReports;
	std::vector<CharacterCollisionReport> g_characterCollisionReports;
	ContactReportCallback g_contactReportCallback;
	CCTHitCallback g_cctHitCallback;

	#define PVD_HOST "127.0.0.1"

	void Init() {
		g_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_allocator, gErrorCallback);
		g_pvd = physx::PxCreatePvd(*g_foundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		g_pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
		g_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_foundation, physx::PxTolerancesScale(), true, g_pvd);
		physx::PxSceneDesc sceneDesc(g_physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		g_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = g_dispatcher;
		sceneDesc.filterShader = contactReportFilterShader;
		sceneDesc.simulationEventCallback = &g_contactReportCallback;

		g_scene = g_physics->createScene(sceneDesc);
		g_scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		g_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);

		physx::PxPvdSceneClient* pvdClient = g_scene->getScenePvdClient();
		if (pvdClient) {
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		g_defaultMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.6f);
		g_grassMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.6f);
		//g_grassMaterial = g_physics->createMaterial(0.8f, 0.7f, 0.05f);

		// Character controller shit
		g_characterControllerManager = PxCreateControllerManager(*g_scene);

		// temporary ground plane
		PxRigidStatic* groundPlane = NULL;
		PxShape* groundShape = NULL;

		groundPlane = PxCreatePlane(*g_physics, PxPlane(0, 1, 0, 0.0f), *g_defaultMaterial);
		g_scene->addActor(*groundPlane);
		groundPlane->getShapes(&groundShape, 1);
		PxFilterData filterData;
		filterData.word0 = RaycastGroup::RAYCAST_ENABLED; // must be disabled or it causes crash in scene::update when it tries to retrieve rigid body flags from this actor
		filterData.word1 = CollisionGroup::ENVIROMENT_OBSTACLE;
		filterData.word2 = CollisionGroup::GENERIC_BOUNCEABLE | CollisionGroup::PLAYER;
		groundShape->setQueryFilterData(filterData);
		groundShape->setSimulationFilterData(filterData); // sim is for ragz
	}

	void BeginFrame() {

	}

	void StepPhysics(float deltaTime) {
		ClearCollisionReports();
		g_scene->simulate(deltaTime);
		g_scene->fetchResults(true);
	}

	void AddCollisionReport(CollisionReport& collisionReport) {
		g_collisionReports.push_back(collisionReport);

		auto* entityA = reinterpret_cast<entt::entity*>(collisionReport.rigidA->userData);
		auto* entityB = reinterpret_cast<entt::entity*>(collisionReport.rigidB->userData);
		std::cout << "Collision detected between " << (int)*entityA << " and " << (int)*entityB << "\n";
	}

	void ClearCollisionReports() {
		g_collisionReports.clear();
	}

	std::vector<CollisionReport>& GetCollisionReports() {
		return g_collisionReports;
	}

	std::vector<CharacterCollisionReport>& GetCharacterCollisionReports() {
		return g_characterCollisionReports;
	}

	PxPhysics* GetPxPhysics()
	{
		return g_physics;
	}

	PxScene* GetPxScene()
	{
		return g_scene;
	}

	CCTHitCallback& GetCharacterControllerHitCallback() {
		return g_cctHitCallback;
	}

	PxControllerManager* GetCharacterControllerManager() {
		return g_characterControllerManager;
	}

	PxMaterial* GetDefaultMaterial()
	{
		return g_defaultMaterial;
	}

	PxShape* CreateBoxShape(float width, float height, float depth, Transform shapeOffset, PxMaterial* material) {
		if (material == NULL) {
			material = Physics::GetDefaultMaterial();
		}
		PxShape* shape = Physics::GetPxPhysics()->createShape(PxBoxGeometry(width, height, depth), *material, true);
		PxMat44 localShapeMatrix = GlmMat4ToPxMat44(shapeOffset.GetWorldMatrix());
		PxTransform localShapeTransform(localShapeMatrix);
		shape->setLocalPose(localShapeTransform);
		return shape;
	}

	PxRigidDynamic* CreateRigidDynamic(Transform transform, PhysicsFilterData filterData, PxShape* shape, Transform shapeOffset) {
		
		PxQuat quat = GlmQuatToPxQuat(transform.GetLocalRotation());
		PxTransform trans = PxTransform(PxVec3(transform.GetLocalPosition().x, transform.GetLocalPosition().y, transform.GetLocalPosition().z), quat);
		PxRigidDynamic* body = Physics::GetPxPhysics()->createRigidDynamic(trans);

		PxFilterData pxFilterData;
		pxFilterData.word0 = (PxU32)filterData.raycastGroup;
		pxFilterData.word1 = (PxU32)filterData.collisionGroup;
		pxFilterData.word2 = (PxU32)filterData.collidesWith;
		shape->setQueryFilterData(pxFilterData);
		shape->setSimulationFilterData(pxFilterData);
		//PxMat44 localShapeMatrix = GlmMat4ToPxMat44(shapeOffset.GetLocalMatrix());
		//PxTransform localShapeTransform(localShapeMatrix);
		//shape->setLocalPose(localShapeTransform);

		body->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		Physics::GetPxScene()->addActor(*body);

		return body;
	}

	PxShape* CreateConvexShapeFromVertexList(std::vector<Vertex>& vertices) {
		PxPhysics* pxPhysics = GetPxPhysics();
		PxMaterial* defaultMaterial = GetDefaultMaterial();

		std::vector<PxVec3> pxVertices;
		for (Vertex& vertex : vertices) {
			pxVertices.push_back(Physics::GlmVec3toPxVec3(vertex.position));
		}

		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = pxVertices.size();
		convexDesc.points.stride = sizeof(PxVec3);
		convexDesc.points.data = pxVertices.data();
		convexDesc.flags = PxConvexFlag::eSHIFT_VERTICES | PxConvexFlag::eCOMPUTE_CONVEX;
		//  s
		PxTolerancesScale scale;
		PxCookingParams params(scale);

		PxDefaultMemoryOutputStream buf;
		PxConvexMeshCookingResult::Enum result;
		if (!PxCookConvexMesh(params, convexDesc, buf, &result)) {
			std::cout << "some convex mesh shit failed\n";
			return 0;
		}
		PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		PxConvexMesh* convexMesh = pxPhysics->createConvexMesh(input);
		PxConvexMeshGeometryFlags flags(~PxConvexMeshGeometryFlag::eTIGHT_BOUNDS);
		PxConvexMeshGeometry geometry(convexMesh, PxMeshScale(PxVec3(1.0f)), flags);

		PxShape* pxShape = pxPhysics->createShape(geometry, *defaultMaterial);
		return pxShape;
	}
}

void CCTHitCallback::onShapeHit(const PxControllerShapeHit& hit) {
	CharacterCollisionReport report;
	report.hitNormal = Faint::Physics::PxVec3toGlmVec3(hit.worldNormal);
	report.worldPosition = Faint::Physics::PxVec3toGlmVec3(hit.worldPos);
	report.characterController = hit.controller;
	report.hitShape = hit.shape;
	report.hitActor = hit.actor;
	Faint::Physics::g_characterCollisionReports.push_back(report);
}

void CCTHitCallback::onControllerHit(const PxControllersHit& hit) {
}

void CCTHitCallback::onObstacleHit(const PxControllerObstacleHit& hit) {
}