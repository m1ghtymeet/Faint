#include "Transform.h"
#include "Util/Util.h"

Faint::Transform::Transform(Vec3 localPosition, Quat localRotation, Vec3 localScale)
	: m_parent(nullptr)
{
	GenerateMatricesLocal(localPosition, localRotation, localScale);
}

Faint::Transform::~Transform()
{
	GenerateMatricesLocal(m_worldPosition, m_worldRotation, m_worldScale);
	m_parent = nullptr;
	UpdateWorldMatrix();
}

Faint::Transform::Transform(const Transform& other) :
	Transform(other.m_worldPosition, other.m_worldRotation, other.m_worldScale)
{

}

//Faint::Transform& Faint::Transform::operator=(const Transform& other)
//{
//	GenerateMatricesWorld(
//		other.m_worldPosition,
//		other.m_worldRotation,
//		other.m_worldScale
//	);
//
//	return *this;
//}

void Faint::Transform::SetParent(Transform& parent)
{
	m_parent = &parent;
}

bool Faint::Transform::RemoveParent()
{
	if (m_parent != nullptr)
	{
		m_parent = nullptr;
		UpdateWorldMatrix();

		return true;
	}
	return false;
}

bool Faint::Transform::HasParent() const
{
	return m_parent != nullptr;
}

void Faint::Transform::GenerateMatricesWorld(Vec3 position, Quat rotation, Vec3 scale)
{
	glm::mat4 m = glm::translate(glm::mat4(1), position);
	m *= glm::mat4_cast(rotation);
	m = glm::scale(m, scale);

	m_worldMatrix = m;
	m_worldPosition = position;
	m_worldRotation = rotation;
	m_worldScale = scale;

	UpdateWorldMatrix();
}

void Faint::Transform::GenerateMatricesLocal(Vec3 position, Quat rotation, Vec3 scale)
{
	glm::mat4 m = glm::translate(glm::mat4(1), position);
	m *= glm::mat4_cast(rotation);
	m = glm::scale(m, scale);

	m_localMatrix = m;
	m_localPosition = position;
	m_localRotation = rotation;
	m_localScale = scale;

	UpdateLocalMatrix();
}

void Faint::Transform::UpdateWorldMatrix()
{
	m_worldMatrix = HasParent() ? m_parent->m_worldMatrix * m_localMatrix : m_localMatrix;
	
	Vec3 outPosition;
	Quat outRotation;
	Vec3 outScale;
	Math::DecomposeTransform(m_worldMatrix, outPosition, outRotation, outScale);
	
	//m_worldPosition = outPosition;
	//m_worldRotation = outRotation;
	//m_worldScale = outScale;

	//UpdateWorldMatrix();
}

void Faint::Transform::UpdateLocalMatrix()
{
	m_localMatrix = HasParent() ? glm::inverse(m_parent->m_worldMatrix) * m_worldMatrix : m_worldMatrix;
	
	Vec3 outPosition;
	Quat outRotation;
	Vec3 outScale;
	Math::DecomposeTransform(m_localMatrix, outPosition, outRotation, outScale);
	
	//m_localPosition = outPosition;
	//m_localRotation = outRotation;
	//m_localScale = outScale;

	//UpdateWorldMatrix();
}

void Faint::Transform::SetLocalPosition(const Vec3& newPosition)
{
	GenerateMatricesLocal(newPosition, m_localRotation, m_localScale);
}

void Faint::Transform::SetLocalRotation(Quat newRotation)
{
	GenerateMatricesLocal(m_localPosition, newRotation, m_localScale);
}

void Faint::Transform::SetLocalScale(Vec3 newScale)
{
	GenerateMatricesLocal(m_localPosition, m_localRotation, newScale);
}

void Faint::Transform::SetWorldPosition(Vec3 newPosition)
{
	GenerateMatricesWorld(newPosition, m_localRotation, m_localScale);
}

void Faint::Transform::SetWorldRotation(Quat newRotation)
{
	GenerateMatricesWorld(m_localPosition, newRotation, m_localScale);
}

void Faint::Transform::SetWorldScale(Vec3 newScale)
{
	GenerateMatricesWorld(m_localPosition, m_localRotation, newScale);
}

Faint::Vec3 Faint::Transform::GetLocalPosition() const
{
	return m_localPosition;
}

Faint::Quat Faint::Transform::GetLocalRotation() const
{
	return m_localRotation;
}

Faint::Vec3 Faint::Transform::GetLocalScale() const
{
	return m_localScale;
}

Faint::Vec3 Faint::Transform::GetWorldPosition() const
{
	return m_worldPosition;
}

Faint::Quat Faint::Transform::GetWorldRotation() const
{
	return m_worldRotation;
}

Faint::Vec3 Faint::Transform::GetWorldScale() const
{
	return m_worldScale;
}

void Faint::Transform::SetLocalMatrix(Matrix4 newMatrix)
{
	m_localMatrix = newMatrix;
}

Faint::Matrix4 Faint::Transform::GetLocalMatrix() const
{
	return m_localMatrix;
}

void Faint::Transform::SetWorldMatrix(Matrix4 newMatrix)
{
	m_worldMatrix = newMatrix;
}

Faint::Matrix4 Faint::Transform::GetWorldMatrix() const
{
	return m_worldMatrix;
}
