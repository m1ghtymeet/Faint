#pragma once
#include "CCollider.h"
#include <AssetManagment/Serializable.h>

#include <memory>

namespace Moon {
	namespace Physics {
		class CharacterController;
	}
	class CharacterControllerComponent : public ColliderComponent {
	public:
		CharacterControllerComponent(Entity& p_owner);
		~CharacterControllerComponent();

		std::string GetName() override;

		std::unique_ptr<Physics::CharacterController>& GetCharacterController();

		bool IsGrounded() const;

		json Serialize();

		void Deserialize(const json& j);

	private:
		std::unique_ptr<Physics::CharacterController> m_characterController;
	};
}