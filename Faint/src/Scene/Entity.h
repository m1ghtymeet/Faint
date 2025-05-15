#pragma once

#include "Common/UUID.h"
#include "Scene.h"
#include "Components/NameComponent.h"
#include "Components.h"

#include "entt.hpp"
#include <type_traits>

namespace Faint {

	class Entity : public ISerializable {
	public:
		Entity();
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other);

		void AddChild(Entity ent);
		bool EntityContainsItself(Entity a, Entity b);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has component");
			T& component = m_scene->Reg().emplace<T>(_entity, std::forward<Args>(args)...);
			m_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			T& component = m_scene->_registry.emplace_or_replace<T>(_entity, std::forward<Args>(args)...);
			m_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent() {
			T& component = m_scene->Reg().get<T>(_entity);
			return component;
		}

		template<typename T>
		T GetComponent() const {
			T component = m_scene->Reg().get<T>(_entity);
			return component;
		}

		template<typename T>
		bool HasComponent() const {
			//return m_scene->Reg().has<T>(_entity);
			return m_scene->Reg().all_of<T>(_entity);
		}

		template<typename T>
		void RemoveComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			m_scene->Reg().remove<T>(_entity);
		}

		void Destroy()
		{
			m_scene->Reg().destroy(_entity);
		}

		operator bool() const { return _entity != entt::null; }
		operator entt::entity() const { return _entity; }
		operator uint32_t() const { return (uint32_t)_entity; }

		UUID GetUUID() { return GetComponent<NameComponent>().id; }
		const std::string& GetName() { return GetComponent<NameComponent>().name; }
		int GetHandle() const { return (int)_entity; }
		int GetID() const { return (int)GetComponent<NameComponent>().id; }
		bool IsValid() const {
			return _entity != (entt::entity)-1 && m_scene->_registry.valid((entt::entity)GetHandle());
		}

		bool operator==(const Entity& other) const {
			return _entity == other._entity && m_scene == other.m_scene;
		}
		bool operator!=(const Entity& other) const {
			return !(*this == other);
		}

		json Serialize() override;
		bool Deserialize(const json& str) override;
		bool DeserializeComponents(const json& str);

		Scene* GetScene() const {
			return m_scene;
		}
	private:
		entt::entity _entity{entt::null};
		Scene* m_scene = nullptr;
	};
}