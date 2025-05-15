#include "TransformComponent.h"
#include <iostream>

namespace Faint
{
	TransformComponent::TransformComponent()
	{
		m_transform = CreateRef<Transform>();
		//std::cout << "A\n";
		m_transform->m_localMatrix = glm::mat4(1);
		m_transform->m_localPosition = Vec3(0.0f);
		m_transform->m_localRotation = Quat();
		m_transform->m_localScale = Vec3(1.0f);

		m_transform->m_worldMatrix = glm::mat4(1);
		m_transform->m_worldPosition = Vec3(0.0f);
		m_transform->m_worldRotation = Quat();
		m_transform->m_worldScale = Vec3(1.0f);
	}

	TransformComponent::TransformComponent(const Vec3& position, const Quat& rotation, const Vec3& scale)
	{
		m_transform->GenerateMatricesLocal(position, rotation, scale);
	}

	Matrix4 TransformComponent::GetGlobalMatrix() const
	{
		return m_transform->GetWorldMatrix();
		//return GlobalMatrix;
	}

	void TransformComponent::SetGlobalMatrix(const Matrix4& transform)
	{
		m_transform->SetWorldMatrix(transform);
		//GlobalMatrix = transform;
	}

	Matrix4 TransformComponent::GetLocalMatrix() const
	{
		return m_transform->GetLocalMatrix();
		//return LocalMatrix;
	}

	void TransformComponent::SetLocalMatrix(const Matrix4& matrix)
	{
		m_transform->SetLocalMatrix(matrix);
		//LocalMatrix = matrix;
	}

	void TransformComponent::SetGlobalPosition(const Vec3& newPosition)
	{
		m_transform->SetWorldPosition(newPosition);
	}

	void TransformComponent::SetGlobalRotation(const Quat& quat)
	{
		m_transform->SetWorldRotation(quat);
		Dirty = true;
	}

	void TransformComponent::SetGlobalScale(const Vec3& newScale)
	{
		m_transform->SetWorldScale(newScale);
		Dirty = true;
	}

	Vec3 TransformComponent::GetLocalPosition() const
	{
		return m_transform->GetLocalPosition();
	}

	Quat TransformComponent::GetLocalRotation() const
	{
		return m_transform->GetLocalRotation();
	}

	Vec3 TransformComponent::GetLocalScale() const
	{
		return m_transform->GetLocalScale();
	}

	Vec3 TransformComponent::GetGlobalPosition() const
	{
		return m_transform->GetWorldPosition();
	}

	Quat TransformComponent::GetGlobalRotation() const
	{
		return m_transform->GetWorldRotation();
	}

	Vec3 TransformComponent::GetGlobalScale() const
	{
		return m_transform->GetWorldScale();
	}

	Transform& TransformComponent::GetTransform()
	{
		return *m_transform.get();
	}

	void TransformComponent::SetParent(Transform& parent) {

		m_transform->SetParent(parent);
	}

	bool TransformComponent::RemoveParent()
	{
		return m_transform->RemoveParent();
	}

	bool TransformComponent::HasParent() const
	{
		return m_transform->HasParent();
	}

	void TransformComponent::SetLocalPosition(const Vec3& newPosition)
	{
		m_transform->SetLocalPosition(newPosition);
		Dirty = true;
	}

	void TransformComponent::SetLocalRotation(const Quat& newRotation)
	{
		m_transform->SetLocalRotation(glm::normalize(newRotation));
		Dirty = true;
	}

	void TransformComponent::SetLocalScale(const Vec3& newScale)
	{
		m_transform->SetLocalScale(newScale);
	}

	json TransformComponent::Serialize()
	{
		Vec3 LocalPosition = m_transform->GetLocalPosition();
		Vec3 WorldPosition = m_transform->GetWorldPosition();
		Quat LocalRotation    = m_transform->GetLocalRotation();
		Quat WorldRotation    = m_transform->GetWorldRotation();
		Vec3 LocalScale    = m_transform->GetLocalScale();
		Vec3 WorldScale    = m_transform->GetWorldScale();
		BEGIN_SERIALIZE();
		SERIALIZE_VEC3(LocalPosition);
		SERIALIZE_VEC3(WorldPosition);
		SERIALIZE_VEC3(LocalRotation);
		SERIALIZE_VEC3(WorldRotation);
		SERIALIZE_VEC3(LocalScale);
		SERIALIZE_VEC3(WorldScale);
		END_SERIALIZE();
	}

	bool TransformComponent::Deserialize(const json& j)
	{
		if (j.contains("LocalPosition"))
		{
			float x = j["LocalPosition"]["x"];
			float y = j["LocalPosition"]["y"];
			float z = j["LocalPosition"]["z"];
			m_transform->SetLocalPosition(Vec3(x, y, z));
		}
		if (j.contains("WorldPosition"))
		{
			float x = j["WorldPosition"]["x"];
			float y = j["WorldPosition"]["y"];
			float z = j["WorldPosition"]["z"];
			m_transform->SetWorldPosition(Vec3(x, y, z));
		}
		if (j.contains("LocalRotation"))
		{
			float x = j["LocalRotation"]["x"];
			float y = j["LocalRotation"]["y"];
			float z = j["LocalRotation"]["z"];
			m_transform->SetLocalRotation(Quat(Vec3(x, y, z)));
		}
		if (j.contains("WorldRotation"))
		{
			float x = j["WorldRotation"]["x"];
			float y = j["WorldRotation"]["y"];
			float z = j["WorldRotation"]["z"];
			m_transform->SetWorldRotation(Vec3(x, y, z));
		}
		return true;
	}
}