#pragma once
#include "AComponent.h"

#include "Math/Math.h"
#include "AssetManagment/Serializable.h"
#include "Math/Transform.h"

namespace Faint {
	class Entity;

	class TransformComponent : public AComponent {
	public:
		/*
		* Create a transform without setting a parent
		* @param p_position
		* @param p_rotation
		* @param p_scale
		*/
		TransformComponent(Entity& p_owner, const Vec3& p_position = Vec3(0.0f, 0.0f, 0.0f), const Quat& p_rotation = Quat(0, 0, 0, 0), const Vec3& p_scale = Vec3(1.0f, 1.0f, 1.0f));

		/**
		* Returns the name of the component
		*/
		std::string GetName() override { return "Transform"; }

		Matrix4 GetLocalMatrix() const;
		void SetLocalMatrix(const Matrix4& matrix);

		Matrix4 GetGlobalMatrix() const;
		void SetGlobalMatrix(const Matrix4& matrix);

		void SetParent(Transform& parent);
		bool RemoveParent();
		bool HasParent() const;

		void SetLocalPosition(const Vec3& newPosition);
		void SetLocalRotation(const Quat& newRotation);
		void SetLocalScale(const Vec3& newScale);

		void SetGlobalPosition(const Vec3& newPosition);
		void SetGlobalRotation(const Quat& newRotation);
		void SetGlobalScale(const Vec3& newScale);

		Vec3 GetLocalPosition() const;
		Quat GetLocalRotation() const;
		Vec3 GetLocalScale() const;

		Vec3 GetGlobalPosition() const;
		Quat GetGlobalRotation() const;
		Vec3 GetGlobalScale() const;
		Matrix4 PreviousMatrix;

		Transform& GetTransform();

		/**
		* Serialize the component
		*/
		json Serialize();
		
		/**
		* Deserialize the component
		*/
		bool Deserialize(const json& str);

	private:
		Transform m_transform;
	};
}