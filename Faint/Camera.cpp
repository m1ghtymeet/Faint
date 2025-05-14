#include "hzpch.h"
#include "Camera.h"

#include "Common/Types.h"
#include "Input/KeyCodes.h"
#include "Input/Input.h"
#include "Core/Application.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

namespace Faint {

	Matrix4 Camera::GetTransformRotation() {
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(Quat(Rotation));
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
		return /*glm::lookAt(Vec3(), Direction, Up)*/m_ViewMatrix;
	}

	void Camera::SetTransform(const Matrix4& transform) {
		m_ViewMatrix = transform;
	}

	Vec3 Camera::GetTranslation()
	{
		return Translation;
	}

	Vec3 Camera::GetDirection()
	{
		return Direction;
	}

	Vec3 Camera::GetUp()
	{
		return Up;
	}

	void Camera::SetPerspective(float fieldOfView, float nearPlane, float farPlane) {

		_fieldOfView = fieldOfView;
		_nearPlane = nearPlane;
		_farPlane = farPlane;
		RecalculateProjection();
	}
	void Camera::SetViewportSizee(float width, float height) {

		_viewportWidth = width;
		_viewportHeight = height;
		RecalculateProjection();
	}
	void Camera::RecalculateProjection() {

		_projectionMatrix = glm::perspective(glm::radians(_fieldOfView), _viewportWidth / _viewportHeight, _nearPlane, _farPlane);
	}
}
