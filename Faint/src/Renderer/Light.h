#pragma once
#include <Math/Transform.h>

namespace Moon::Rendering {
	enum LightType { Point, Directional, Spot };

	struct LightData {
		int type; // 4 bytes
		float intensity; // 4 bytes
		float colorR; float colorG; float colorB; // 4 bytes
		float padding[3]; // 12 bytes
		glm::mat4 data; // 64 bytes
	};

	// Point light specific data stored in mat4
	struct PointLightData {
		float posX; float posY; float posZ;
		float constant;
		float linear;
		float quadratic;
	};

	struct DirectionalLightData {
		float directX; float directY; float directZ;
	};

	struct Light {
		Transform transform;
		LightType type = LightType::Point;
		glm::vec3 color{ 1.00f, 0.7799999713897705f, 0.5289999842643738f };
		float strength = 1.0f;

		// Point light properties
		float radius = 16.0f;
		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;

		// Directional light properties
		glm::vec3 direction{ 0.0f, 0.0f, 0.0f };

		// Spot light properties
		float cutoff = 12.0f;
		float outerCutoff = 15.0f;

		// Shadow properties
		bool castShadows = true;
		float shadowAreaSize = 50.0f;
		bool shadowFollowCamera = true;
		int16_t shadowMapResolution = 2048;

		bool dirty = true;

		glm::mat4 CalculateMatrix() const;
		glm::vec3 GetDirectionFromQuat(glm::quat q) const;

		LightData PackLightData(LightType type, const glm::vec3& worldPosition, const glm::quat& worldRotation) const;
	};
}