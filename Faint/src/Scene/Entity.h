#pragma once

#include <unordered_map>
#include <memory>

#include "Core/Base.h"
#include "Common/UUID.h"

#include "Components/AComponent.h"
#include "Components/CTransform.h"
#include "AssetManagment/Serializable.h"
#include "Event/Event.h"

#include <entt.hpp>
#include <type_traits>

namespace Faint {
	class Scene;
	/**
	* The Entity is the main class of the ECS, it corresponds to the entity and is
	* composed of components scripts
	*/
	class Entity : public ISerializable {
	public:
		/**
		* Constructor of the actor
		* @param p_gbID
		* @param p_name
		* @param p_tag
		* @param p_playing
		*/
		Entity(int64_t p_gbID, const std::string& p_name, const std::string& p_tag, bool& p_playing);
		/**
		* Constructor of the entity. It will automatically add a name, visibility and transform component
		* @param handle
		* @param scene
		*/
		Entity(entt::entity handle, Scene* scene, bool& p_playing);
		//Entity(const Entity& other);
		~Entity();

		/**
		* Return the current name of the gameobject
		*/
		const std::string& GetName() const;

		/**
		* Defines a new name for the gameobject
		* @param p_name
		*/
		void SetName(const std::string& p_name);

		/**
		* Enable or disable the gameobject
		* @param p_active
		*/
		void SetActive(bool p_active);

		/**
		* Returns true if the gameobject is active, ignoring his parent (if any) active state
		*/
		bool IsSelfActive() const;

		/**
		* Returns true if the gameobject and his recursive parents (if any) are active
		*/
		bool IsActive() const;

		/**
		* Defines a new ID for the gameobject
		* @param p_id
		*/
		void SetID(int64_t p_id);

		/**
		* Returns the ID of the gameobject
		*/
		int64_t GetID() const;

		/**
		* Returns the Handle of the gameobject
		*/
		int GetHandle() const { return (int)_entity; }

		/**
		* Set an gameobject as the parent of this actor
		* @param p_parent
		*/
		void SetParent(Entity& p_parent);

		/**
		* Detach from the parent
		*/
		void DetachFromParent();

		/**
		* Returns true if the gameobject has a parent
		*/
		bool HasParent() const;

		/**
		* Returns the parents of this gameobject (Or nullptr if no parent)
		*/
		Entity* GetParent() const;

		/**
		* Returns the children of this gameobject
		*/
		std::vector<Entity*>& GetChildren();

		/**
		* Mark the GameObject as "Destroyed". A "Destroyed" gameobject will be removed from the scene by the scene itself
		*/
		void Destroy();

		/**
		* Defines if the gameobject is sleeping or not.
		* A sleeping gameobject will not trigger methods suchs as OnEnable, OnDisable and OnDestroy
		* @param p_sleeping
		*/
		void SetSleeping(bool p_sleeping);

		/**
		* Called when the scene start or when the gameobject 
		* This method will always be called in an ordered triple.
		* - OnAwake()
		* - OnEnable()
		* - OnStart()
		*/
		void OnAwake();

		/**
		* Add a component to the gameobject (Or return the component if already existing)
		* @param p_args (Paramter pack forwared to the component constructor)
		*/
		template<typename T, typename ... Args>
		T& AddComponent(Args&&... p_args);

		/**
		* Remove the given component
		*/
		template<typename T>
		bool RemoveComponent();

		/**
		* Try to get the given component (Returns nullptr on failure)
		*/
		template<typename T>
		T* GetComponent() const;

		/**
		* Returns a refrence to the vector of components
		*/
		std::vector<Ref<AComponent>>& GetComponents();

		//Scene* GetScene() const { return m_scene; }

		/**
		* Serialize all the components
		*/
		json Serialize() override;

		/**
		* Deserialize all the components
		*/
		void Deserialize(const json& str) override;

		//TransformComponent& transform;

		/* Deleted */
		operator bool() const { return m_id != 0; }
	private:
		/**
		* @brief Deleted copy constructor
		* @param p_entity
		*/
		//Entity(const Entity& p_entity) = delete;

	public:
		/* Some events that are triggeren when an action occur on the gameobject instance */
		Event<AComponent&> ComponentAddedEvent;
		Event<AComponent&> ComponentRemovedEvent;

		/* Some events that are triggered when an action occur on any gameobject */
		static Event<Entity&> CreatedEvent;
	private:
		/* Settings */
		std::string m_name;
		std::string m_tag;
		bool		m_active = true;
		bool&		m_playing;
		
		/* Internal settings */
		int64_t		m_id;
		bool		m_destroyed = false;
		bool		m_sleeping = true;
		bool		m_awaked = false;
		bool		m_started = false;

		/* Parenting system stuff */
		int64_t				 m_parentID = 0;
		Entity*				 m_parent = nullptr;
		std::vector<Entity*> m_children;

		/* Entities components */
		std::vector<Ref<AComponent>> m_components;

		entt::entity _entity{entt::null};
		Scene* m_scene = nullptr;
	};
}

#include "Scene/Entity.inl"