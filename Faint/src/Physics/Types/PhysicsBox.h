#pragma once

#include <Physics/PhysicsActor.h>

namespace Moon::Physics {

	class PhysicsBox : public PhysicsActor {
	public:
		PhysicsBox(Transform& transform, const glm::vec3& halfExtents = glm::vec3(0.5f, 0.5f, 0.5f), float density = 1.0f);

		void CreateActor() override;
		void Reshape(const glm::vec3& halfExtents = glm::vec3(1.0f), const glm::vec3& offsetPos = glm::vec3(0), const glm::quat& offsetRot = glm::quat{});

		void SetShapeOffset(const glm::vec3& offsetPos, const glm::quat& offsetRot = glm::quat());

		void CreateShape() override;

	protected:
		void UpdatePhysxTransform() override;
		void UpdateTransform() override;

	private:
		glm::vec3 m_halfExtents;
		glm::vec3 m_shapeOffset = glm::vec3(0);
		glm::vec3 m_lastScale = glm::vec3(1.0f);
		float m_density;
	};
}