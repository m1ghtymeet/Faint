#pragma once

#include "Component.h"
#include "Renderer/Types/Mesh.h"

namespace Faint {

	class SpriteComponent : public Component {
		
		//HAZELCOMPONENT(BoxColliderComponent, "Box Collider");

		static void InitializeComponentClass() {
			//BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
			//BindComponentFieldProperty<&BoxColliderComponent::SetSize>("IsTrigger", "Is Trigger");
		}
	public:
		bool Billboard;
		bool LockYRotation;
		bool PositionFacing;

		std::string SpritePath;
		Ref<Mesh> SpriteMesh;

		SpriteComponent();

		void SetSprite(std::string path);
		bool LoadSprite();

		json Serialize();
		bool Deserialize(const json& j);
	};
}