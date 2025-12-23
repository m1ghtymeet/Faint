#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <functional>

namespace Moon {

	class Transform {
	public:
		Transform(glm::vec3 localPosition = glm::vec3(0), glm::quat localRotation = glm::quat(), glm::vec3 localScale = glm::vec3(1));
		~Transform();
	
		Transform(const Transform& other);
	
		Transform& operator=(const Transform& other);
	
		//void NotificationHandler(TransformNotifier::Notification notification);
	
		void SetParent(Transform& parent);
		bool RemoveParent();
		bool HasParent() const;
	
		void GenerateMatricesWorld(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
		void GenerateMatricesLocal(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
		void UpdateWorldMatrix();
		void UpdateLocalMatrix();
	
		void SetLocalPosition(const glm::vec3& newPosition);
		void SetLocalRotation(glm::quat newRotation);
		void SetLocalScale(glm::vec3 newScale);
	
		void SetWorldPosition(glm::vec3 newPosition);
		void SetWorldRotation(glm::quat newRotation);
		void SetWorldScale(glm::vec3 newScale);

		void SetRotationEuler(const glm::vec3& euler);
		glm::vec3 GetRotationEuler() const;
	
		glm::vec3 GetLocalPosition() const;
		glm::quat GetLocalRotation() const;
		glm::vec3 GetLocalScale() const;
	
		glm::vec3 GetWorldPosition() const;
		glm::quat GetWorldRotation() const;
		glm::vec3 GetWorldScale() const;
	
		void SetLocalMatrix(glm::mat4 newMatrix);
		glm::mat4 GetLocalMatrix() const;
	
		void SetWorldMatrix(glm::mat4 newMatrix);
		glm::mat4 GetWorldMatrix() const;
	
	private:
		glm::vec3 m_localPosition;
		glm::quat m_localRotation;
		glm::vec3 m_localScale;
		glm::vec3 m_worldPosition;
		glm::quat m_worldRotation;
		glm::vec3 m_worldScale;
		glm::vec3 m_eulerRotation = glm::vec3(0.0f);
	
		glm::mat4 m_localMatrix;
		glm::mat4 m_worldMatrix;
	
		Transform* m_parent;
		std::vector<Transform*> m_children;
	};

	//struct Transform {
	//	glm::vec3 position = glm::vec3(0);
	//	glm::vec3 rotation = glm::vec3(0);
	//	glm::vec3 scale = glm::vec3(1);
	//
	//	Transform() = default;
	//	explicit Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
	//		this->position = position;
	//		this->rotation = rotation;
	//		this->scale = scale;
	//	}
	//
	//	glm::mat4 to_mat4() {
	//		glm::mat4 m = glm::translate(glm::mat4(1), position);
	//		m *= glm::mat4_cast(glm::quat(rotation));
	//		m = glm::scale(m, scale);
	//		return m;
	//	}
	//	glm::vec3 to_forward_vector() {
	//		glm::quat q = glm::quat(rotation);
	//		return glm::normalize(q * glm::vec3(0.0f, 0.0f, 1.0f));
	//	}
	//	glm::vec3 to_right_vector() {
	//		glm::quat q = glm::quat(rotation);
	//		return glm::normalize(q * glm::vec3(1.0f, 0.0f, 0.0f));
	//	}
	//};
}