#pragma once

#include "Physics/Physics.h"

namespace Faint {
	//namespace Physics { class CharacterController; }
	class CharacterControllerComponent {
	private:
		//Ref<Physics::CharacterController> m_CharacterController;
		PxController* m_characterController;

	public:
		// Auto stepping settings
		bool AutoStepping = false;
		glm::vec3 StickToFloorStepDown = glm::vec3(0.f, -0.5f, 0.f);
		glm::vec3 StepDownExtra = glm::vec3(0, 0, 0); // ??
		glm::vec3 SteppingStepUp = glm::vec3(0.f, 0.4f, 0.f);
		float SteppingMinDistance = 0.125f;
		float SteppingForwardDistance = 0.250f;

		float Friction = 0.5f;
		float MaxSlopeAngle = 45.0f;
		float MaxStrength = 1.0f;

		CharacterControllerComponent() {};

		void CreateCharacterController() {

		}
		void SetCharacterController(PxController* controller) {
			m_characterController = controller;
		}
		PxController* GetCharacterController() const {
			return m_characterController;
		}
		//void SetCharacterController(const Ref<Physics::CharacterController>& charController) {
		//	m_CharacterController = charController;
		//}
		//
		//Ref<Physics::CharacterController> GetCharacterController() const {
		//	return m_CharacterController;
		//}
	};
}