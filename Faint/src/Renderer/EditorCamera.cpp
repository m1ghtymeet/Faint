#include "hzpch.h"
#include "EditorCamera.h"

#include "Engine.h"
#include "Input/Input.h"
#include "Input/KeyCodes.h"
#include "Common/Types.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Faint {

	float yaw = 1.0f;
	float pitch = 1.0f;
	float m_TargetYaw = 0.0f;
	float m_TargetPitch = 0.0f;

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		SetProjectionMatrix(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip));
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		SetProjectionMatrix(glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip));
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		SetPosition(CalculatePosition());

		glm::quat orientation = GetOrientation();
		//AddPitch(m_Pitch);
		//AddYaw(m_Yaw);
		SetRotationQ(orientation);
		Update();
		//m_ViewMatrix = glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(orientation);
		//m_ViewMatrix = glm::inverse(m_ViewMatrix);
		
		//Update();
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Time ts) {
		if (Input::KeyDown(Key::LeftAlt)) {
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (Input::RightMouseDown())
				MousePan(delta);
			else if (Input::LeftMouseDown())
				MouseRotate(delta);
			else if (Input::MouseWheelUp() || Input::MouseWheelDown())
				MouseZoom(delta.y);
		}
		else {
			if (Input::RightMouseDown()) {
				Input::DisableCursor();

				//const bool smoothCamera = /*Engine::GetProject()->Settings.*/true;
				//yaw = glm::mix(yaw, m_TargetYaw, smoothCamera ? 1.0f : 1.0f);
				//pitch = glm::mix(pitch, m_TargetPitch, smoothCamera ? 1.0f : 1.0f);
				//AddPitch(pitch);
				//AddYaw(yaw);

				//float diffx = Input::GetMouseXOffset();
				//float diffy = Input::GetMouseYOffset();
				//diffx *= 0.1f;
				//diffy *= 0.1f;
				//
				//m_TargetYaw += diffx;
				//m_TargetPitch += diffy;
			}
			else {
				Input::ShowCursor();
			}
		}

		float delta = 0;
		if (Input::MouseWheelDown())
			delta = -1 * 0.1f;
		else if (Input::MouseWheelUp())
			delta = 1 * 0.1f;
		MouseZoom(delta);
		UpdateView();
	}

	//void EditorCamera::OnEvent(Event& e)
	//{
	//	EventDispatcher dispatcher(e);
	//	dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	//}
	//
	//bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	//{
	//	float delta = e.GetYOffset() * 0.1f;
	//	MouseZoom(delta);
	//	UpdateView();
	//	return false;
	//}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRight() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUp() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUp().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForward();
			m_Distance = 1.0f;
		}
	}

	//glm::vec3 EditorCamera::GetUpDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	//}
	//
	//glm::vec3 EditorCamera::GetRightDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	//}
	//
	//glm::vec3 EditorCamera::GetForwardDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	//}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)) * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}