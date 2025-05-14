#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "Scene/Camera2.h"

namespace Faint {

	struct CameraComponent : public Component {
		//HAZELCOMPONENT(CameraComponent, "Camera");
	public:
		Ref<Camera> camera;
		TransformComponent* transform;
		bool primary = true;
		bool fixedAspectRatio = false;

		CameraComponent() { camera = CreateRef<Camera>(); }

		json Serialize() {
			BEGIN_SERIALIZE();
			//SERIALIZE_OBJECT(camera);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			
			return true;
		}
	};
}