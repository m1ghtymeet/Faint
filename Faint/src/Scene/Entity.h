#pragma once

#include <unordered_map>
#include <memory>

#include "Components/AComponent.h"
#include "Components/CTransform.h"
#include "Components/CCollider.h"
#include "Components/Behaviour.h"
#include "AssetManagment/Serializable.h"
#include "Event/Event.h"

namespace Moon {
	class Scene;
	/**
	* The Entity is the main class of the ECS, it corresponds to the entity and is
	* composed of components scripts
	*/
	class Entity : public ISerializable {
	public:
		/**
		* Constructor of the entity
		* @param p_gbID
		* @param p_name
		* @param p_tag
		* @param p_playing
		*/
		Entity(int64_t p_gbID, const std::string& p_name, const std::string& p_tag, bool& p_playing);

		/**
		* Desctructor
		*/
		~Entity();

		/**
		* Return the current name of the gameobject
		*/
		const std::string& GetName() const;

		/**
		* Return the current tag of the gameobject
		*/
		const std::string& GetTag() const;

		/**
		* Defines a new name for the gameobject
		* @param p_name
		*/
		void SetName(const std::string& p_name);

		/**
		* Defines a new tag for the gameobject
		* @param p_name
		*/
		void SetTag(const std::string& p_tag);

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

		Entity* GetChildByName(const std::string& name) const;
		//Entity* FindChildByName(const std::string& name, bool recursive = true) const;

		void AddChild(Entity* child);
		void RemoveChild(Entity* child);

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
		* Called when the scene start or when the gameobject
		* This method will always be called in an ordered triple.
		* - OnAwake()
		* - OnEnable()
		* - OnStart()
		*/
		void OnStart();

		/**
		* Called when the scene start or when the gameobject
		* This method will always be called in an ordered triple.
		* - OnAwake()
		* - OnEnable()
		* - OnStart()
		*/
		void OnEnable();

		/**
		* Called when the gameobject hierarchical active state changed to false or gets destroyed while beign hierarchically active.
		* Conditions:
		* - Play mode only
		*/
		void OnDisable();

		/**
		* Called when the actor gets destroyed if it has been awaked
		* Conditions:
		* - Play mode only
		*/
		void OnDestroy();

		/**
		* Called every frame
		* @param p_deltaTime
		*/
		void OnUpdate(float p_deltaTime);

		void OnCollisionEnter(Moon::ColliderComponent& otherBody);
		void OnCollisionStay(Moon::ColliderComponent& otherBody);
		void OnCollisionExit(Moon::ColliderComponent& otherBody);

		void OnTriggerEnter(Moon::ColliderComponent& otherBody);
		void OnTriggerStay(Moon::ColliderComponent& otherBody);
		void OnTriggerExit(Moon::ColliderComponent& otherBody);

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
		std::vector<std::shared_ptr<AComponent>>& GetComponents();

		Behaviour& AddBehaviour(const std::string& p_name, const std::string& p_path = "");

		bool RemoveBehaviour(Behaviour& p_behaviour);
		
		bool RemoveBehaviour(const std::string& p_name);

		Behaviour* GetBehaviour(const std::string& p_name);

		std::unordered_map<std::string, Behaviour>& GetBehaviours();

		/**
		* Serialize all the components
		*/
		json Serialize() override;

		/**
		* Deserialize all the components
		*/
		void Deserialize(const json& str) override;

		void RecursiveActiveUpdate();

		void RecursiveWasActiveUpdate();

		TransformComponent* transform = nullptr;

		/* Deleted */
		operator bool() const { return m_id != 0; }
	private:
		/**
		* @brief Deleted copy constructor
		* @param p_entity
		*/
		Entity(const Entity& p_entity) = delete;

	public:
		/* Some events that are triggeren when an action occur on the gameobject instance */
		Event<AComponent&> ComponentAddedEvent;
		Event<AComponent&> ComponentRemovedEvent;

		/* Some events that are triggered when an action occur on any gameobject */
		static Event<Entity&> CreatedEvent;
		static Event<Entity&> DestroyedEvent;
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
		bool		m_wasActive = false;

		/* Parenting system stuff */
		int64_t				 m_parentID = 0;
		Entity*				 m_parent = nullptr;
		std::vector<Entity*> m_children;

		/* Entities components */
		std::vector<std::shared_ptr<AComponent>> m_components;
		std::unordered_map<std::string, Behaviour> m_behaviours;
	};
}

#include "Scene/Entity.inl"