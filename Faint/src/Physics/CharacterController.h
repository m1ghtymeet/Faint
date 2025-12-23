#pragma once

#include <glm/glm.hpp>
#include <Math/Types.h>
#include <Math/Transform.h>
#include <Physics/PhysicsActor.h>
#include "characterkinematic/PxController.h"

namespace physx {
	class PxController;
	class PxShape;
	class PxControllerManager;
}
namespace Moon::Physics {

	struct CharacterControllerSettings {
		float height = 1.9f;
		float crouchHeight = 1.0f;
		float radius = 0.4f;
		float stepOffset = 0.45f;
		float contactOffset = 0.02f;
		float skinWidth = 0.08f;
		float slopeLimitDeg = 50.0f;
		float scaleCoeff = 0.9f;
		physx::PxMaterial* material = nullptr;
	};

	class CharacterController : public PhysicsActor {
	public:
		CharacterController(Transform& transform, CharacterControllerSettings settings = {});
		~CharacterController();

		void CreateActor() override {};
		void Create(const CharacterControllerSettings& settings);

		void Move(const glm::vec3& displacement);

		bool IsGrounded() const;

		void SetPosition(glm::vec3 position);
		glm::vec3 GetPosition() const;

		void SetFootPosition(glm::vec3 position);
		glm::vec3 GetFootPosition() const;

		physx::PxController* GetController() const;

		void SetSettings(CharacterControllerSettings settings) { m_settings = settings; }
		inline CharacterControllerSettings& GetSettings() { return m_settings; }
		inline const CharacterControllerSettings& GetSettings() const { return m_settings; }
		void Recreate();

	private:
		void CreateController();
		void UpdateGroundState();

		virtual void CreateShape() override {};
		virtual void UpdatePhysxTransform() override {};
		virtual void UpdateTransform() override;
	public:
		Event<PhysicsActor&> TriggerEnterEvent;
		Event<PhysicsActor&> TriggerStayEvent;
		Event<PhysicsActor&> TriggerExitEvent;
	private:
		// Settings
		CharacterControllerSettings m_settings;
		float m_currentHeight = 0.0f;
		float m_targetHeight = 0.0f;
		float m_heightVelocity = 0.0f;
		
		// Status
		bool m_isGrounded = false;
		bool m_wasGroundedLastFrame = false;
		physx::PxControllerCollisionFlags m_lastMoveFlags{ 0 };

		// PhysX
		physx::PxController* m_controller = nullptr;
		physx::PxShape* m_shape = nullptr;
	};
}