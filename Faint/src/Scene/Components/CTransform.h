#pragma once
#include "AComponent.h"

#include <Math/Types.h>
#include "AssetManagment/Serializable.h"
#include "Math/Transform.h"

#include <AssetManagment/Helper/Serializer.h>

namespace Moon {

	class Entity;

	/**
	* Represents the 3D transformations applied to an entity
	*/
	class TransformComponent : public AComponent {
	public:
		/*
		* Create a transform without setting a parent
		* @param p_position
		* @param p_rotation
		* @param p_scale
		*/
		TransformComponent(Entity& p_owner, const glm::vec3& p_position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::quat& p_rotation = glm::quat(0, 0, 0, 0), const glm::vec3& p_scale = glm::vec3(1.0f, 1.0f, 1.0f));

		/**
		* Returns the name of the component
		*/
		std::string GetName() override;

		/**
		* Defines a parent to the transform
		* @param p_parent
		*/
		void SetParent(TransformComponent& p_parent);
		
		/**
		* Set the parent to nullptr and recalculate world matrix
		* Returns true on success
		*/
		bool RemoveParent();

		/**
		* Check if the transform has a parent
		*/
		bool HasParent() const;

		/**
		* Set the position of the transform in the local space
		* @param p_newPosition
		*/
		void SetLocalPosition(glm::vec3 p_newPosition);
		
		/**
		* Set the rotation of the transform in the local space
		* @param p_newRotation
		*/
		void SetLocalRotation(const glm::quat& p_newRotation);

		/**
		* Set the scale of the transform in the local space
		* @param p_newScale
		*/
		void SetLocalScale(glm::vec3 p_newScale);

		/**
		* Set the matrix of the transform in the local space
		* @param p_newMatrix
		*/
		void SetLocalMatrix(const glm::mat4& p_newMatrix);

		/**
		* Set the position of the transform in the world space
		* @param p_newPosition
		*/
		void SetGlobalPosition(glm::vec3 p_newPosition);
		
		/**
		* Set the rotation of the transform in the world space
		* @param p_newRotation
		*/
		void SetGlobalRotation(glm::quat p_newRotation);
		
		/**
		* Set the scale of the transform in the world space
		* @param p_newScale
		*/
		void SetGlobalScale(glm::vec3 p_newScale);

		/**
		* Set the matrix of the transform in the world space
		* @param p_newMatrix
		*/
		void SetGlobalMatrix(const glm::mat4& p_newMatrix);

		/**
		* Return the position in local space
		*/
		const glm::vec3& GetLocalPosition() const;
		
		/**
		* Return the rotation in local space
		*/
		const glm::quat& GetLocalRotation() const;

		/**
		* Return the scale in local space
		*/
		const glm::vec3& GetLocalScale() const;

		/**
		* Return the matrix in local space
		*/
		const glm::mat4& GetLocalMatrix() const;

		/**
		* Return the position in world space
		*/
		const glm::vec3& GetGlobalPosition() const;
		
		/**
		* Return the position in world space
		*/
		const glm::quat& GetGlobalRotation() const;
		
		/**
		* Return the position in world space
		*/
		const glm::vec3& GetGlobalScale() const;
	
		/**
		* Return the matrix in world space
		*/
		const glm::mat4& GetGlobalMatrix() const;

		/**
		* Return the Transform attached to the TransformComponent
		*/
		Transform& GetTransform();

		/**
		* Serialize the component
		*/
		json Serialize();
		
		/**
		* Deserialize the component
		* @param str
		*/
		void Deserialize(const json& str);

		bool m_dirty = false;

	private:
		Transform m_transform;
	};
}