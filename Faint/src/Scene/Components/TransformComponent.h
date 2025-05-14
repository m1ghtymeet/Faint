#pragma once

#include "Core/Base.h"
#include "Math/Math.h"
#include "AssetManagment/Serializable.h"
#include "Component.h"
#include "Math/Transform.h"

namespace Faint {

	class TransformComponent : public Component {
	public:
		Matrix4 GlobalMatrix;
		Matrix4 LocalMatrix;
	public:
		bool Dirty = true;
		bool GlobalDirty = true;

		TransformComponent();
		TransformComponent(const Vec3& position, const Quat& rotation, const Vec3& scale);

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

		json Serialize();
		bool Deserialize(const json& str);

	private:
		Ref<Transform> m_transform;
	};
}