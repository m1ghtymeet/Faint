#include "Transform.h"
#include "Math.h"
#include <iostream>

Moon::Transform::Transform(glm::vec3 localPosition, glm::quat localRotation, glm::vec3 localScale)
	: m_parent(nullptr)
{
	GenerateMatricesLocal(localPosition, localRotation, localScale);
}

Moon::Transform::~Transform()
{
	//GenerateMatricesLocal(m_worldPosition, m_worldRotation, m_worldScale);
	m_parent = nullptr;
	//UpdateWorldMatrix();
}

Moon::Transform::Transform(const Transform& other) :
	Transform(other.m_worldPosition, other.m_worldRotation, other.m_worldScale)
{

}

Moon::Transform& Moon::Transform::operator=(const Transform& other)
{
	GenerateMatricesWorld(
		other.m_worldPosition,
		other.m_worldRotation,
		other.m_worldScale
	);

	return *this;
}

void Moon::Transform::SetParent(Transform& parent) {
	if (m_parent)
		m_parent->m_children.erase(
			std::remove(m_parent->m_children.begin(), m_parent->m_children.end(), this),
			m_parent->m_children.end()
		);
	m_parent = &parent;
	parent.m_children.push_back(this);
	UpdateWorldMatrix();
}

bool Moon::Transform::RemoveParent() {
	if (m_parent != nullptr) {
		//auto& siblings = m_parent->m_children;
		//siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
		m_parent = nullptr;
		UpdateWorldMatrix();
		return true;
	}
	return false;
}

bool Moon::Transform::HasParent() const
{
	return m_parent != nullptr;
}

void Moon::Transform::GenerateMatricesWorld(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
	glm::mat4 m = glm::translate(glm::mat4(1), position);
	m *= glm::mat4_cast(glm::normalize(rotation));
	m = glm::scale(m, scale);

	m_worldMatrix = m;
	m_worldPosition = position;
	m_worldRotation = rotation;
	m_worldScale = scale;

	UpdateLocalMatrix();
	//UpdateWorldMatrix();
}

void Moon::Transform::GenerateMatricesLocal(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
	glm::mat4 m = glm::translate(glm::mat4(1), position);
	m *= glm::mat4_cast(glm::normalize(rotation));
	m = glm::scale(m, scale);
	
	m_localMatrix = m;
	m_localPosition = position;
	m_localRotation = rotation;
	m_localScale = scale;

	UpdateWorldMatrix();
	//if (m_parent) m_parent->UpdateWorldMatrix();
}

void Moon::Transform::UpdateWorldMatrix()
{
	m_worldMatrix = HasParent() ? m_parent->m_worldMatrix * m_localMatrix : m_localMatrix;
	
	m_worldPosition.x = m_worldMatrix[3][0];
	m_worldPosition.y = m_worldMatrix[3][1];
	m_worldPosition.z = m_worldMatrix[3][2];

	glm::vec3 columns[3] =
	{
		{ m_worldMatrix[0][0], m_worldMatrix[1][0], m_worldMatrix[2][0] },
		{ m_worldMatrix[0][1], m_worldMatrix[1][1], m_worldMatrix[2][1] },
		{ m_worldMatrix[0][2], m_worldMatrix[1][2], m_worldMatrix[2][2] },
	};

	m_worldScale.x = glm::length(columns[0]);
	m_worldScale.y = glm::length(columns[1]);
	m_worldScale.z = glm::length(columns[2]);

	if (m_worldScale.x) {
		columns[0] /= m_worldScale.x;
	}
	if (m_worldScale.y) {
		columns[1] /= m_worldScale.y;
	}
	if (m_worldScale.z) {
		columns[2] /= m_worldScale.z;
	}

	glm::mat3 rotationMatrix
	(
		columns[0].x, columns[1].x, columns[2].x,
		columns[0].y, columns[1].y, columns[2].y,
		columns[0].z, columns[1].z, columns[2].z
	);

	m_worldRotation = glm::quat(rotationMatrix);

	for (Transform* child : m_children)
		child->UpdateWorldMatrix();
}

void Moon::Transform::UpdateLocalMatrix()
{
	m_localMatrix = HasParent() ? glm::inverse(m_parent->m_worldMatrix) * m_worldMatrix : m_worldMatrix;
	
	m_localPosition.x = m_localMatrix[3][0];
	m_localPosition.y = m_localMatrix[3][1];
	m_localPosition.z = m_localMatrix[3][2];

	glm::vec3 columns[3] =
	{
		{ m_localMatrix[0][0], m_localMatrix[1][0], m_localMatrix[2][0] },
		{ m_localMatrix[0][1], m_localMatrix[1][1], m_localMatrix[2][1] },
		{ m_localMatrix[0][2], m_localMatrix[1][2], m_localMatrix[2][2] },
	};

	m_localScale.x = glm::length(columns[0]);
	m_localScale.y = glm::length(columns[1]);
	m_localScale.z = glm::length(columns[2]);

	if (m_localScale.x) {
		columns[0] /= m_localScale.x;
	}
	if (m_localScale.y) {
		columns[1] /= m_localScale.y;
	}
	if (m_localScale.z) {
		columns[2] /= m_localScale.z;
	}

	glm::mat3 rotationMatrix
	(
		columns[0].x, columns[1].x, columns[2].x,
		columns[0].y, columns[1].y, columns[2].y,
		columns[0].z, columns[1].z, columns[2].z
	);

	m_localRotation = glm::quat(rotationMatrix);

	//for (Transform* child : m_children)
	//	child->UpdateLocalMatrix();
}

void Moon::Transform::SetLocalPosition(const glm::vec3& newPosition)
{
	GenerateMatricesLocal(newPosition, m_localRotation, m_localScale);
}

void Moon::Transform::SetLocalRotation(glm::quat newRotation)
{
	GenerateMatricesLocal(m_localPosition, newRotation, m_localScale);
	SetRotationEuler(Math::QuaternionToEulerDegrees(newRotation));
}

void Moon::Transform::SetLocalScale(glm::vec3 newScale)
{
	GenerateMatricesLocal(m_localPosition, m_localRotation, newScale);
}

void Moon::Transform::SetWorldPosition(glm::vec3 newPosition)
{
	GenerateMatricesWorld(newPosition, m_worldRotation, m_worldScale);
}

void Moon::Transform::SetWorldRotation(glm::quat newRotation)
{
	GenerateMatricesWorld(m_worldPosition, newRotation, m_worldScale);
	SetRotationEuler(Math::QuaternionToEulerDegrees(newRotation));
}

void Moon::Transform::SetWorldScale(glm::vec3 newScale)
{
	GenerateMatricesWorld(m_worldPosition, m_worldRotation, newScale);
}

void Moon::Transform::SetRotationEuler(const glm::vec3& euler) {
	m_eulerRotation = euler;
}

glm::vec3 Moon::Transform::GetRotationEuler() const {
	return m_eulerRotation;
}

glm::vec3 Moon::Transform::GetLocalPosition() const
{
	return m_localPosition;
}

glm::quat Moon::Transform::GetLocalRotation() const
{
	return m_localRotation;
}

glm::vec3 Moon::Transform::GetLocalScale() const
{
	return m_localScale;
}

glm::vec3 Moon::Transform::GetWorldPosition() const
{
	return m_worldPosition;
}

glm::quat Moon::Transform::GetWorldRotation() const
{
	return m_worldRotation;
}

glm::vec3 Moon::Transform::GetWorldScale() const
{
	return m_worldScale;
}

void Moon::Transform::SetLocalMatrix(glm::mat4 newMatrix)
{
	m_localMatrix = newMatrix;
}

glm::mat4 Moon::Transform::GetLocalMatrix() const
{
	return m_localMatrix;
}

void Moon::Transform::SetWorldMatrix(glm::mat4 newMatrix)
{
	m_worldMatrix = newMatrix;
}

glm::mat4 Moon::Transform::GetWorldMatrix() const
{
	return m_worldMatrix;
}
