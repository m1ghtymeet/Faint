#pragma once
#pragma warning(push, 0)
#include "PxPhysicsAPI.h"
#include <geometry/PxGeometryHelpers.h>
#pragma warning(pop)
#include "CollisionReports.h"
#include "Common/Types.h"
#include "Math/Transform.h"
#include "Types/RigidDynamic.h"
#include <span>
#include <vector>

namespace Faint::Physics {
	void Init();
	void BeginFrame();
	void StepPhysics(float deltaTime);
	void SubmitDebugLinesToRenderer(const DebugRenderMode& debugRenderMode);
	void AddCollisionReport(CollisionReport& collisionReport);
	void ClearCollisionReports();
	void ClearCharacterControllerCollisionReports();
	std::vector<CollisionReport>& GetCollisionReports();
	std::vector<CharacterCollisionReport>& GetCharacterCollisionReports();
	PxPhysics* GetPxPhysics();
	PxScene* GetPxScene();
	CCTHitCallback& GetCharacterControllerHitCallback();
	PxControllerManager* GetCharacterControllerManager();

	// Materials
	PxMaterial* GetDefaultMaterial();

	// Create
	PxShape* CreateBoxShape(float width, float height, float depth, Transform shapeOffset = Transform(), PxMaterial* material = NULL);
	PxRigidDynamic* CreateRigidDynamic(Transform transform, PhysicsFilterData filterData, PxShape* shape, Transform shapeOffset = Transform());
	PxShape* CreateConvexShapeFromVertexList(std::vector<Vertex>& vertices);

	// Rigid Dynamics
	void UpdateActiveRigidDynamicAABBList();
	void RemoveRigidDynamic(uint64_t rigidDynamicId);
	bool RigidDynamicExists(uint64_t rigidDynamicId);
	void RemoveAnyRigidDynamicForRemoval();
	void UpdateAllRigidDynamics(float deltaTime);

	// Destroy
	void Destroy(PxRigidDynamic*& rigidDynamic);
	void Destroy(PxRigidStatic*& rigidStatic);
	void Destroy(PxShape*& shape);
	void Destroy(PxRigidBody*& rigidBody);
	void Destroy(PxTriangleMesh*& triangleMesh);

	// Util
	glm::vec3 PxVec3toGlmVec3(PxVec3 vec);
	glm::vec3 PxVec3toGlmVec3(PxExtendedVec3 vec);
	PxVec3 GlmVec3toPxVec3(glm::vec3 vec);
	PxQuat GlmQuatToPxQuat(glm::quat quat);
	PxMat44 GlmMat4ToPxMat44(glm::mat4 glmMatrix);
	PhysXRayResult CastPhysXRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, PxU32 collisionFlags, bool cullbackFacing = false);
}