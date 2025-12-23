#include "CTransform.h"

namespace Moon
{
	TransformComponent::TransformComponent(Entity& p_owner, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
		: AComponent(p_owner)
	{
		m_transform.GenerateMatricesLocal(position, rotation, scale);
	}

	const glm::mat4& TransformComponent::GetGlobalMatrix() const
	{
		return m_transform.GetWorldMatrix();
	}

	void TransformComponent::SetGlobalMatrix(const glm::mat4& transform)
	{
		m_transform.SetWorldMatrix(transform);
	}

	std::string TransformComponent::GetName() {
		return "Transform";
	}

	const glm::mat4& TransformComponent::GetLocalMatrix() const
	{
		return m_transform.GetLocalMatrix();
	}

	void TransformComponent::SetLocalMatrix(const glm::mat4& matrix)
	{
		m_transform.SetLocalMatrix(matrix);
	}

	void TransformComponent::SetGlobalPosition(glm::vec3 newPosition)
	{
		m_transform.SetWorldPosition(newPosition);
	}

	void TransformComponent::SetGlobalRotation(glm::quat quat)
	{
		m_transform.SetWorldRotation(quat);
	}

	void TransformComponent::SetGlobalScale(glm::vec3 newScale)
	{
		m_transform.SetWorldScale(newScale);
	}

	const glm::vec3& TransformComponent::GetLocalPosition() const
	{
		return m_transform.GetLocalPosition();
	}

	const glm::quat& TransformComponent::GetLocalRotation() const
	{
		return m_transform.GetLocalRotation();
	}

	const glm::vec3& TransformComponent::GetLocalScale() const
	{
		return m_transform.GetLocalScale();
	}

	const glm::vec3& TransformComponent::GetGlobalPosition() const
	{
		return m_transform.GetWorldPosition();
	}

	const glm::quat& TransformComponent::GetGlobalRotation() const
	{
		return m_transform.GetWorldRotation();
	}

	const glm::vec3& TransformComponent::GetGlobalScale() const
	{
		return m_transform.GetWorldScale();
	}

	Transform& TransformComponent::GetTransform()
	{
		return m_transform;
	}

	void TransformComponent::SetParent(TransformComponent& parent) {

		m_transform.SetParent(parent.GetTransform());
	}

	bool TransformComponent::RemoveParent()
	{
		return m_transform.RemoveParent();
	}

	bool TransformComponent::HasParent() const
	{
		return m_transform.HasParent();
	}

	void TransformComponent::SetLocalPosition(glm::vec3 newPosition)
	{
		m_transform.SetLocalPosition(newPosition);
	}

	void TransformComponent::SetLocalRotation(const glm::quat& newRotation)
	{
		m_transform.SetLocalRotation(newRotation);
	}

	void TransformComponent::SetLocalScale(glm::vec3 newScale)
	{
		m_transform.SetLocalScale(newScale);
	}

	json TransformComponent::Serialize() {
		BEGIN_SERIALIZE();

		glm::vec3 LocalPosition = m_transform.GetLocalPosition();
		glm::vec4 LocalRotation = {
			m_transform.GetLocalRotation().x,
			m_transform.GetLocalRotation().y,
			m_transform.GetLocalRotation().z,
			m_transform.GetLocalRotation().w
		};
		glm::vec3 LocalScale    = m_transform.GetLocalScale();
		SERIALIZE_VEC3(LocalPosition);
		j["LocalRotation"] = { {"x", LocalRotation.x}, {"y", LocalRotation.y}, {"z", LocalRotation.z} , {"w", LocalRotation.w} };
		SERIALIZE_VEC3(LocalScale);

		//glm::vec3 WorldPosition = m_transform.GetWorldPosition();
		//glm::quat WorldRotation    = m_transform.GetWorldRotation();
		//glm::vec3 WorldScale    = m_transform.GetWorldScale();
		//SERIALIZE_VEC3(WorldPosition);
		//SERIALIZE_VEC3(WorldRotation);
		//SERIALIZE_VEC3(WorldScale);
		END_SERIALIZE();
	}

	void TransformComponent::Deserialize(const json& j) {
		SetLocalPosition({ j["LocalPosition"]["x"], j["LocalPosition"]["y"], j["LocalPosition"]["z"] });
		glm::quat loadedRotation(
			j["LocalRotation"]["w"],
			j["LocalRotation"]["x"],
			j["LocalRotation"]["y"],
			j["LocalRotation"]["z"]
		);
		SetLocalRotation(loadedRotation);
		SetLocalScale(glm::vec3(j["LocalScale"]["x"], j["LocalScale"]["y"], j["LocalScale"]["z"]));
	}
}