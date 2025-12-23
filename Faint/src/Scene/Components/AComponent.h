#pragma once

#include <string>

namespace Moon {
	class Entity;
	class ColliderComponent;

	/**
	* AComponent is theh base class for any component
	* A component is a set of data and scripts (Entity-Component without systems) that is interpreted by the engine (Or the user)
	*/
	class AComponent {
	public:
		/**
		* Constructor of a AComponent (Must be called by derived classes)
		* @param p_owner
		*/
		AComponent(Entity& p_owner);
		~AComponent();

		/**
		* Called when the scene start right before OnStart
		* It allows you to apply prioritized game logic on scene start
		*/
		virtual void OnAwake() {}

		/**
		* Called when the scene start right after OnAwake
		* It allows you to apply prioritized game logic on scene start
		*/
		virtual void OnStart() {}

		/**
		* Called when the components gets enabled (owner SetActive set to true) and after OnAwake() on scene starts
		*/
		virtual void OnEnable() {}

		/**
		* 
		*/
		virtual void OnDisable() {}

		/**
		* Called when the components gets destroyed
		*/
		virtual void OnDestroy() {}

		/**
		* Called every frame
		* @param p_deltaTime
		*/
		virtual void OnUpdate(float p_deltaTime) {}

		virtual void OnCollisionEnter(ColliderComponent& otherBody) {}
		virtual void OnCollisionStay(ColliderComponent& otherBody) {}
		virtual void OnCollisionExit(ColliderComponent& otherBody) {}

		virtual void OnTriggerEnter(ColliderComponent& otherBody) {}
		virtual void OnTriggerStay(ColliderComponent& otherBody) {}
		virtual void OnTriggerExit(ColliderComponent& otherBody) {}

		/**
		* Returns the name of the component
		*/
		virtual std::string GetName() = 0;

		Entity& owner;
	};
}