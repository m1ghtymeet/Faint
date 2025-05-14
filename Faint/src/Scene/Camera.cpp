#include "Camera2.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include "Core/Window.h"

namespace Faint {

	void Camera::Update() {
		m_inverseViewMatrix = glm::translate(glm::mat4(1), m_position) * glm::mat4_cast(m_rotationQ);
		m_viewMatrix = glm::inverse(m_inverseViewMatrix);
		m_right = glm::vec3(m_inverseViewMatrix[0]);
		m_up = glm::vec3(m_inverseViewMatrix[1]);
		m_forward = glm::vec3(m_inverseViewMatrix[2]);
	}

	void Camera::AddPitch(float value) {
		m_rotation.x += value;
		m_rotation.x = std::clamp(m_rotation.x, m_minPitch, m_maxPitch);
		m_rotationQ = glm::quat(m_rotation);
		Update();
	}

	void Camera::AddYaw(float value) {
		m_rotation.y += value;
		Update();
	}

	void Camera::SetPosition(const glm::vec3& position) {
		m_position = position;
	}

	void Camera::SetRotation(const glm::vec3& rotation) {
		m_rotation = rotation;
		m_rotationQ = glm::quat(rotation);
	}

	void Camera::SetRotationQ(const glm::quat& rotation) {
		m_rotationQ = rotation;
	}

	void Camera::SetProjectionMatrix(const glm::mat4& proj) {
		m_projectionMatrix = proj;
	}

	void Camera::SetViewMatrix(const glm::mat4& view) {
		m_viewMatrix = view;
	}

	const glm::mat4& Faint::Camera::GetProjectionMatrix() const {
		return m_projectionMatrix;
	}

	const glm::mat4& Camera::GetViewMatrix() const {
		return m_viewMatrix;
	}

	const glm::mat4& Camera::GetInverseViewMatrix() const {
		return m_inverseViewMatrix;
	}

	const glm::vec3& Camera::GetPosition() const {
		return m_position;
	}

	const glm::quat& Camera::GetEulerRotation() const {
		return m_rotationQ;
	}

	const glm::vec3& Camera::GetForward() const {
		return m_forward;
	}

	const glm::vec3& Camera::GetUp() const {
		return m_up;
	}

	const glm::vec3& Camera::GetRight() const {
		return m_right;
	}
}