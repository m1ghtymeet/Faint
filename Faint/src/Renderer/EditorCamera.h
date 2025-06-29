#pragma once

#include "Scene/Camera.h"
#include "Core/Time.h"

#include <glm/glm.hpp>

namespace Faint {

	/**
	* A simple camera
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

		void OnUpdate(Time ts);
	};

}