#include "Entity.h"

namespace Moon {

	template<typename T, typename ...Args>
	inline T& Entity::AddComponent(Args&& ...p_args) {
		static_assert(std::is_base_of<AComponent, T>::value, "T should derive from AComponent!");

		if (auto found = GetComponent<T>(); !found) {
			m_components.insert(m_components.begin(), std::make_shared<T>(*this, p_args...));
			T& instance = *dynamic_cast<T*>(m_components.front().get());
			ComponentAddedEvent.Invoke(instance);
			if (m_playing && IsActive()) {
				reinterpret_cast<AComponent&>(instance).OnAwake();
				reinterpret_cast<AComponent&>(instance).OnEnable();
				reinterpret_cast<AComponent&>(instance).OnStart();
			}
			return instance;
		}
		else {
			return *found;
		}
	}

	template<typename T>
	inline bool Entity::RemoveComponent() {
		static_assert(std::is_base_of<AComponent, T>::value, "T should derive from AComponent!");
		static_assert(!std::is_same<TransformComponent, T>::value, "You can't remove a Transform Component from an entity!");
		
		std::shared_ptr<T> result(nullptr);
		
		for (auto it = m_components.begin(); it != m_components.end(); it++) {
			result = std::dynamic_pointer_cast<T>(*it);
			if (result) {
				ComponentRemovedEvent.Invoke(*result.get());
				m_components.erase(it);
				return true;
			}
		}
		
		return false;
	}

	template<typename T>
	inline T* Entity::GetComponent() const {
		static_assert(std::is_base_of<AComponent, T>::value, "T should derive from AComponent!");
		
		std::shared_ptr<T> result(nullptr);
		
		for (auto it = m_components.begin(); it != m_components.end(); it++) {
			result = std::dynamic_pointer_cast<T>(*it);
			if (result) {
				return result.get();
			}
		}
		
		return nullptr;
	}

	//template<typename T, typename ...Args>
	//inline T& Entity::AddOrReplaceComponent(Args && ...p_args) {
	//	T& component = m_scene->_registry.emplace_or_replace<T>(_entity, std::forward<Args>(p_args)...);
	//	m_scene->OnComponentAdded<T>(*this, component);
	//	return component;
	//}
}