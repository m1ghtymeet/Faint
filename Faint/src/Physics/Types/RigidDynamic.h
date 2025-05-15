#pragma once
#include "PxShape.h"
#include "PxRigidDynamic.h"
#include <glm/vec3.hpp>

using namespace physx;

struct RigidDynamic {
	void Update(float deltaTime);
	void ActivatePhysics();
	void DeactivatePhysics();
	void MarkForRemoval();
	void SetPxRigidDynamic(PxRigidDynamic* rigidDynamic);
	void SetPxShape(PxShape* shape);

private:
	PxShape* m_pxShape = nullptr;
	PxRigidDynamic* m_pxRigidDynamic = nullptr;
	glm::vec3 m_currentPosition;
	glm::vec3 m_previousPosition;
	float m_stationaryTime = 0;
	bool m_activePhysics = false;
	bool m_markedForRemoval = false;
};