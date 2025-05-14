#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Faint {
	struct Camera {
	public:
		void Update();

		void AddPitch(float value);
		void AddYaw(float value);
		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation);
		void SetRotationQ(const glm::quat& rotation);
		void SetProjectionMatrix(const glm::mat4& proj);
		void SetViewMatrix(const glm::mat4& view);
		const glm::mat4& GetProjectionMatrix() const;
		const glm::mat4& GetViewMatrix() const;
		const glm::mat4& GetInverseViewMatrix() const;
		const glm::vec3& GetPosition() const;
		const glm::quat& GetEulerRotation() const;
		const glm::vec3& GetForward() const;
		const glm::vec3& GetUp() const;
		const glm::vec3& GetRight() const;

		float m_fieldOfView = 1.0f;
		float m_nearPlane = 0.1f;
		float m_farPlane = 100.0f;
		glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	private:
		glm::vec3 m_position = glm::vec3(0.0f);
		glm::vec3 m_rotation = glm::vec3(0.0f);
		glm::quat m_rotationQ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_inverseViewMatrix = glm::mat4(1.0f);
		
		float m_minPitch = -1.5f;
		float m_maxPitch = 1.5f;
	};
}