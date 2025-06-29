#pragma once

#include "Core/Time.h"
#include "Common/UUID.h"
#include "Renderer/EditorCamera.h"

#include "Scene/Systems/PhysicsSystem.h"
#include "Scene/Systems/TransformSystem.h"
#include "Scene/Systems/ScriptingSystem.h"
#include "Entity.h"

#include "Components/CModelRenderer.h"
#include "Components/CCamera.h"

#include <entt.hpp>

namespace Faint {
	class PhysicsSystem;

	enum SelectedType {
		NONE = 0,
		_CAMERA,
		_LIGHT,
		OTHER
	};

	/**
	* The scene is a set of gameobjects
	*/
	class Scene : public ISerializable {
	private:

		std::vector<Ref<System>> m_systems;
	public:
		Ref<ScriptingSystem> m_ScriptingSystem;
		
		struct FastAccessComponents {
			std::vector<MeshRendererComponent*> modelRenderers;
			std::vector<CameraComponent*> cameras;
		};

		/**
		* Constructor of the scene
		*/
		Scene();

		/**
		* Handle the memory de-allocation of every gameobjects
		*/
		~Scene();

		/**
		* Play the scene
		*/
		void Play();

		/**
		* Update every gameobjects
		* @param time
		*/
		void Update(Time time);

		/**
		* Update every gameobjects 60 frames per seconds
		*/
		void FixedUpdate(Time time);

		/**
		* @param name
		*/
		std::string GetUniqueEntityName(const std::string& name);

		/**
		* Create an gameobject with a default name and return a refrence to it.
		*/
		Entity& CreateEntity(std::string name = "New Entity");

		/**
		* Create an gameobject with a default name and return a refrence to it.
		* @param p_name
		* @param p_tag
		*/
		Entity& CreateEntity(const std::string& p_name, const std::string& p_tag);

		/**
		* Destroy and gameobject and return true on success
		* @param p_entity
		*/
		void DestroyEntity(Entity& p_entity);

		/**
		* Return the first gameobject identified by the given name, or nullptr on fail
		* @paam p_name
		*/
		Entity* GetEntityByName(const std::string& p_name) const;

		/**
		* Return the first gameobject identified by the given tag, or nullptr on fail
		* @param p_tag
		*/
		Entity* GetEntityByTag(const std::string& p_tag) const;

		/**
		* Return the gameobject identified by the given ID (Returns nullptr on fail)
		* @param p_id
		*/
		Entity* GetEntityByID(int64_t p_id) const;

		std::string FullPath;

		bool OnInit();
		void OnExit();
		void EditorUpdate(Time time);

		void Draw();
		void Draw(const Matrix4& projection, const Matrix4& view);

		static Ref<Scene> Save();

		bool EntityExists(const std::string& name);

		/**
		* Return a refrence on the gameobject map
		*/
		std::vector<Entity*>& GetAllEntities();

		void OnViewportResize(float width, float height);

		Entity DuplicateEntity(Entity entity);

		Ref<Camera> GetCurrentCamera();

		entt::registry& Reg() { return _registry; }

		Entity* GetPrimaryCameraEntity();

		Ref<EditorCamera> m_EditorCamera;
		SelectedType selectedType;

		/**
		* Return the fast access components data structure 
		*/
		const FastAccessComponents& GetFastAccessComponents() const;

		/**
		* Serialize the scene
		*/
		json Serialize() override;

		/**
		* Deserialize the scene
		* @param j
		*/
		void Deserialize(const json& j) override;
	private:
		int64_t m_availableID = 1;
		bool m_isPlaying = false;
		std::vector<Entity*> m_entities;

		FastAccessComponents m_fastAccessComponents;

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry _registry;
		float _viewportWidth = 1280, _viewportHeight = 720;

		float fixedUpdateRate = 1.0f / 90.0f;
		float fixedUpdateDifference = 0;

		std::unordered_map<uint32_t, Entity*> m_EntityIDMap;
		std::unordered_map<std::string, Entity*> m_EntityNameMap;

		friend Entity;
	};
}