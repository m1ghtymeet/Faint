#pragma once

#include "AComponent.h"
#include <Scene/Components/CRigidBody.h>
#include <AssetManagment/Serializable.h>
#include "Math/Transform.h"

namespace Moon {
	class BoxColliderComponent : public RigidBodyComponent {
	public:
		BoxColliderComponent(Entity& p_owner);
		
		std::string GetName() override;
		
		glm::vec3 halfExtents = { 0.5f, 0.5f, 0.5f };
		bool isTrigger = false;
		float density = 1.0f;
		glm::vec3 shapeOffset = glm::vec3(0);

		void SetTrigger(bool trigger);

		void SetSize(const glm::vec3& newSize);

		void SetShapeOffset(const glm::vec3& posOffset, const glm::quat& rotOffset = glm::quat{});

		glm::vec3 GetSize();

		json Serialize();

		void Deserialize(const json& j);

		virtual void Init() override;

	private:
		void OnEnable() override;
		void OnDisable() override;
		void OnUpdate(float p_deltaTime) override;
		bool m_needPhysicsInit = false;
	};
}