#include "Physics.h"

namespace Faint::Physics {

	glm::vec3 PxVec3toGlmVec3(PxVec3 vec) {
		return { vec.x, vec.y, vec.z };
	}

	glm::vec3 PxVec3toGlmVec3(PxExtendedVec3 vec) {
		return { vec.x, vec.y, vec.z };
	}

	PxVec3 GlmVec3toPxVec3(glm::vec3 vec) {
		return { vec.x, vec.y, vec.z };
	}

	PxQuat GlmQuatToPxQuat(glm::quat quat) {
		return { quat.x, quat.y, quat.z, quat.w };
	}

	PxMat44 GlmMat4ToPxMat44(glm::mat4 glmMatrix) {
		PxMat44 matrix;
		std::copy(glm::value_ptr(glmMatrix),
			glm::value_ptr(glmMatrix) + 16,
			reinterpret_cast<float*>(&matrix));
		return matrix;
	}

	PhysXRayResult CastPhysXRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, PxU32 collisionFlags, bool cullbackFacing) {

		return PhysXRayResult();
	}
}