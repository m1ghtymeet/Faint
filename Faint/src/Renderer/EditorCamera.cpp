#include "hzpch.h"
#include "EditorCamera.h"

#include "Engine.h"
#include "Input/Input.h"
#include "Input/KeyCodes.h"
#include "Common/Types.h"

#include "Scene/Components/CModelRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Faint {

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {
		SetProjectionMatrix(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip));
		SetPosition(glm::vec3(0, 0, 5));
		m_fieldOfView = fov;
		m_nearPlane = nearClip;
		m_farPlane = farClip;
	}

	void EditorCamera::UpdateProjection(float width, float height) {
		SetProjectionMatrix(glm::perspective(glm::radians(m_fieldOfView), (float)(width / height), m_nearPlane, m_farPlane));
	}

	float GetEntityFocusDist(Entity& p_entity) {
		float distance = 4.0f;

		if (p_entity.IsActive()) {
			if (auto modelRenderer = p_entity.GetComponent<MeshRendererComponent>()) {
				const bool hasModel = modelRenderer->GetModel();
				const auto& entityPosition = p_entity.GetComponent<TransformComponent>()->GetGlobalPosition();
				const auto& entityScale = p_entity.GetComponent<TransformComponent>()->GetGlobalScale();
				const auto scaleFactor = std::max(std::max(entityScale.x, entityScale.y), entityScale.z);

				distance = std::max(distance, 10.0f);
			}

			for (auto child : p_entity.GetChildren())
				distance = std::max(distance, GetEntityFocusDist(*child));
		}

		return distance;
	}

	
	void EditorCamera::OnUpdate(Time ts) {

		if (Input::RightMouseDown()) {
			Input::HideCursor();
			Input::LockMouse(true);

			AddYaw(-Input::GetMouseXOffset() * 0.1f * ts);
			AddPitch(-Input::GetMouseYOffset() * 0.1f * ts);

			// Free fly movement (WASD)
			glm::vec3 direction(0.0f);
			if (Input::KeyDown(Key::W)) direction -= GetForward();
			if (Input::KeyDown(Key::S)) direction += GetForward();
			if (Input::KeyDown(Key::A)) direction -= GetRight();
			if (Input::KeyDown(Key::D)) direction += GetRight();
			if (Input::KeyDown(Key::Q)) direction -= GetUp();
			if (Input::KeyDown(Key::E)) direction += GetUp();

			SetPosition(GetPosition() + (direction * 1.8f) * (float)ts);
		}
		else {
			//Input::ShowCursor();
			Input::LockMouse(false);
		}

		Update();
	}
}