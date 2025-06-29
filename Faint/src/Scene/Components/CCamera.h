#pragma once

#include "AComponent.h"
#include "CTransform.h"
#include "Scene/Camera.h"

namespace Faint {
	struct CameraComponent : public AComponent {
	public:
		Ref<Camera> camera;
		TransformComponent* transform;
		bool primary = true;
		bool fixedAspectRatio = false;

		CameraComponent(Entity& p_owner) : AComponent(p_owner) { camera = CreateRef<Camera>(); }

		std::string GetName() override { return "Camera"; }

		json Serialize() {
			BEGIN_SERIALIZE();
			j["NearPlane"] = camera->m_nearPlane;
			j["FarPlane"] = camera->m_farPlane;
			j["FOV"] = camera->m_fieldOfView;
			j["FixedAspectRatio"] = fixedAspectRatio;
			j["Primary"] = primary;
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			
			return true;
		}
	};
}