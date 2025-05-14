#include "hzpch.h"
#include "Entity.h"

#include "Components/ParentComponent.h"

namespace Faint {
	Entity::Entity() {
		this->_entity = (entt::entity)-1;
	}
	Entity::Entity(entt::entity handle, Scene* scene)
		: _entity(handle), m_scene(scene) {
	}
	Entity::Entity(const Entity& other) {
		this->_entity = other._entity;
		this->m_scene = other.m_scene;
	}

	void Entity::AddChild(Entity ent) {
		if ((int)_entity != ent.GetHandle()) {
			ent.GetComponent<ParentComponent>().HasParent = true;
			ent.GetComponent<ParentComponent>().Parent = *this;

			GetComponent<ParentComponent>().Children.push_back(ent);
		}
	}

	bool Entity::EntityContainsItself(Entity a, Entity b) {
		ParentComponent& targeParentComponent = b.GetComponent<ParentComponent>();
		if (!targeParentComponent.HasParent)
			return false;

		Entity currentParent = b.GetComponent<ParentComponent>().Parent;
		while (currentParent != a)
		{
			if (currentParent.GetComponent<ParentComponent>().HasParent)
				currentParent = currentParent.GetComponent<ParentComponent>().Parent;
			else
				return false;

			if (currentParent == a)
				return true;
		}
		return true;
	}

	json Entity::Serialize() {

		BEGIN_SERIALIZE();
		SERIALIZE_OBJECT_REF_LBL("NameComponent", GetComponent<NameComponent>());
		SERIALIZE_OBJECT_REF_LBL("ParentComponent", GetComponent<ParentComponent>());
		SERIALIZE_OBJECT_REF_LBL("TransformComponent", GetComponent<TransformComponent>());
		SERIALIZE_OBJECT_REF_LBL("VisibilityComponent", GetComponent<VisibilityComponent>());

		//if (HasComponent<CameraComponent>())
		//	SERIALIZE_OBJECT_REF_LBL(CameraComponent);


		END_SERIALIZE();
	}

	bool Entity::Deserialize(const json& str) {
		return false;
	}

	bool Entity::DeserializeComponents(const json& str)
	{
		return false;
	}
}