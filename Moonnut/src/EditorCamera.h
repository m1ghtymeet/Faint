#pragma once

#include "Scene/Camera.h"
#include "Core/Time.h"

#include <glm/glm.hpp>

namespace Moon {

	/**
	* A simple camera editor
	*/
	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;

		/**
		* Constructor
		* @param fov
		* @param aspectRatio
		* @param nearClip
		* @param farClip
		*/
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void UpdateProjection(float width, float height);

		void HandleInputs(float p_deltaTime);

		void OnUpdate(float p_deltaTime);

		bool m_using = false;

	private:
		bool m_moving = false;
		float m_currentSpeed = 0.0f;
		glm::vec3 m_currentDirection = glm::vec3(0);
		float m_sensitivity = 0.2f;
	};

}