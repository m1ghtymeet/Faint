#pragma once

#include "Math.h"

namespace Faint {

	class Transform {
	public:
		Transform(Vec3 localPosition = Vec3(0), Quat localRotation = Quat(), Vec3 localScale = Vec3(1));
		~Transform();

		Transform(const Transform& other);

		//Transform& operator=(const Transform& other);

		void SetParent(Transform& parent);
		bool RemoveParent();
		bool HasParent() const;

		void GenerateMatricesWorld(Vec3 position, Quat rotation, Vec3 scale);
		void GenerateMatricesLocal(Vec3 position, Quat rotation, Vec3 scale);
		void UpdateWorldMatrix();
		void UpdateLocalMatrix();

		void SetLocalPosition(const Vec3& newPosition);
		void SetLocalRotation(Quat newRotation);
		void SetLocalScale(Vec3 newScale);

		void SetWorldPosition(Vec3 newPosition);
		void SetWorldRotation(Quat newRotation);
		void SetWorldScale(Vec3 newScale);

		Vec3 GetLocalPosition() const;
		Quat GetLocalRotation() const;
		Vec3 GetLocalScale() const;

		Vec3 GetWorldPosition() const;
		Quat GetWorldRotation() const;
		Vec3 GetWorldScale() const;

		void SetLocalMatrix(Matrix4 newMatrix);
		Matrix4 GetLocalMatrix() const;

		void SetWorldMatrix(Matrix4 newMatrix);
		Matrix4 GetWorldMatrix() const;

		Vec3 m_localPosition;
		Quat m_localRotation;
		Vec3 m_localScale;
		Vec3 m_worldPosition;
		Quat m_worldRotation;
		Vec3 m_worldScale;

		Matrix4 m_localMatrix;
		Matrix4 m_worldMatrix;

	private:
		Transform* m_parent;
	};
}