#pragma once

#include "AComponent.h"
#include "CTransform.h"
#include "Scene/Camera.h"

namespace Moon {
	class CameraComponent : public AComponent {
	public:
		Camera camera;

		CameraComponent(Entity& p_owner);

		std::string GetName() override { return "Camera"; }

		void Update(int renderWidth, int renderHeight);

		void MarkAsDirty();

		void SetFov(float p_value);

		void SetNear(float p_value);

		void SetFar(float p_value);

		float GetFov() const;

		float GetNear() const;

		float GetFar() const;

		json Serialize();

		void Deserialize(const json& j);

	private:
		bool dirty = false;
		float m_w = 1.0f, m_h = 1.0f;
	};
}