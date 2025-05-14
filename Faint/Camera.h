#pragma once

#include <glm/glm.hpp>

#include "Core/Time.h"
#include "Math/Math.h"
#include "AssetManagment/Serializable.h"

namespace Faint {

	class Camera : public ISerializable {
	private:
		Vec3 Scale = { 1.0f, 1.0f, 1.0f };
		Vec3 Up = { 0.0f, 1.0f, 0.0f };

	public:
		enum class ProjectionType { Perspective = 0, Orthographic };

		Vec3 Direction = Vec3(0, 0, 1);
		Vec3 Right = Vec3(1, 0, 0);

		Vec3 Translation = { 0.0f, 0.0f, 0.0f };
		Vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		float _viewportWidth = 0, _viewportHeight = 0;

		Camera() = default;
		Camera(const glm::mat4& projection) : _projectionMatrix(projection) {}
		virtual ~Camera() = default;

		Matrix4 GetTransformRotation();
		void SetTransform(const Matrix4& transform);

		Vec3 GetTranslation();
		Vec3 GetDirection();
		Vec3 GetUp();

		void SetPerspective(float fieldOfView, float nearPlane, float farPlane);
		void SetViewportSizee(float width, float height);
		void SetProjectionType(ProjectionType type) { _projectionType = type; }
		void SetFieldOfView(float fov) { _fieldOfView = fov; }
		void SetNearPlane(float nearPlane) { _nearPlane = nearPlane; }
		void SetFarPlane(float farPlane) { _farPlane = farPlane; }

		float GetFieldOfView() { return _fieldOfView; }
		float GetNearPlane() { return _nearPlane; }
		float GetFarPlane() { return _farPlane; }
		ProjectionType GetProjectionType() const { return _projectionType; }

		const glm::mat4& GetProjection() const { return _projectionMatrix; }
		const glm::mat4& GetViewMatrix() { return m_ViewMatrix; }

		json Serialize() {
			BEGIN_SERIALIZE();
			END_SERIALIZE();
		}
		bool Deserialize(const json& j) {
			return true;
		}
	protected:
		Matrix4 _projectionMatrix = Matrix4(1.0f);
		Matrix4 m_ViewMatrix = Matrix4(1.0f);
		ProjectionType _projectionType = ProjectionType::Perspective;

		void RecalculateProjection();
		float _fieldOfView = 1.0f;
		float _nearPlane = 0.1f;
		float _farPlane = 500.0f;

		friend class EditorCamera;
	};
}