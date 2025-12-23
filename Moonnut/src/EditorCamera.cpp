#include "EditorCamera.h"

#include <Scene/Entity.h>
#include <Scene/Components/CModelRenderer.h>
#include "Input/Input.h"
#include "Input/KeyCodes.h"

#define SMALL_NUMBER (float)9.99999993922529e-9

namespace Moon {
	float FInterpTo(float current, float target, float deltaTime, float interpSpeed) {
		if (interpSpeed < 0.0f)
			return target;
		const float dist = target - current;
		if (dist * dist < SMALL_NUMBER)
			return target;
		const float deltaMove = dist * glm::clamp(deltaTime * interpSpeed, 0.0f, 1.0f);
		return current + deltaMove;
	}

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {
		SetProjectionMatrix(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip));
		SetPosition(glm::vec3(0.0f, 1.0f, 3.0f));
		//SetRotation(glm::vec3(-160.0f, 45.0f, 180.0f));
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

	glm::vec3 m_ypr = glm::vec3(0);
	void EditorCamera::HandleInputs(float p_deltaTime) {

		m_using = false;

		if (Input::RightMouseDown()) {
			m_using = true;

			Input::HideCursor();
			Input::LockMouse(true);

			m_ypr.x += -Input::GetMouseYOffset() * m_sensitivity * p_deltaTime;
			m_ypr.y += -Input::GetMouseXOffset() * m_sensitivity * p_deltaTime;
			m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

			SetRotation(m_ypr);

			m_moving = false;
			glm::vec3 targetDirection(0.0f);
			if (Input::KeyDown(Key::W)) { m_moving = true; targetDirection -= GetForward(); }
			if (Input::KeyDown(Key::S)) { m_moving = true; targetDirection += GetForward(); }
			if (Input::KeyDown(Key::A)) { m_moving = true; targetDirection -= GetRight(); }
			if (Input::KeyDown(Key::D)) { m_moving = true; targetDirection += GetRight(); }
			if (Input::KeyDown(Key::Q)) { m_moving = true; targetDirection -= GetUp(); }
			if (Input::KeyDown(Key::E)) { m_moving = true; targetDirection += GetUp(); }

			if (glm::length(targetDirection) > 0.0f)
				targetDirection = glm::normalize(targetDirection);

			float targetSpeed = Input::KeyDown(Key::LeftShift) ? 6.0f : 4.0f;
			if (!m_moving) targetSpeed = 0.0f;

			m_currentDirection = glm::mix(m_currentDirection, targetDirection, glm::clamp(p_deltaTime * 12.0f, 0.0f, 1.0f));
			m_currentSpeed = FInterpTo(m_currentSpeed, targetSpeed, p_deltaTime, 18.0f);

			SetPosition(GetPosition() + (m_currentDirection * m_currentSpeed) * p_deltaTime);
			
			Input::Update();
		}
		else {
			Input::ShowCursor();
			Input::LockMouse(false);
		}
	}
	
	void EditorCamera::OnUpdate(float p_deltaTime) {

		Update();
	}
}