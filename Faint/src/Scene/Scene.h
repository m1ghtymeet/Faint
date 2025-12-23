#pragma once

#include "Entity.h"

#include "Components/CModelRenderer.h"
#include "Components/CSkinnedModelRenderer.h"
#include "Components/CCamera.h"
#include "Components/CLight.h"
#include "Components/CAudio.h"
#include "Components/CText2D.h"
#include "Components/CWidget.h"

namespace Moon {
	/**
	* The scene is a set of gameobjects
	*/
	class Scene : public ISerializable {
	public:
		struct FastAccessComponents {
			std::vector<MeshRendererComponent*> modelRenderers;
			std::vector<SkinnedMeshRendererComp*> skinnedModelRenderers;
			std::vector<CameraComponent*> cameras;
			std::vector<LightComponent*> lights;
			std::vector<AudioComponent*> audios;
			std::vector<Text2DComponent*> texts;
			std::vector<UI::WidgetComponent*> widgets;
		};

		/**
		* Constructor of the scene
		*/
		Scene();

		/**
		* Handle the memory de-allocation of every gameobjects
		*/
		~Scene();

		void AddDefaultCamera();

		void AddDefaultLights();

		/**
		* Play the scene
		*/
		void Play();

		bool IsPlaying() const;

		/**
		* Update every gameobjects
		* @param time
		*/
		void Update(float p_deltaTime);

		/**
		* Update every gameobjects 60 frames per seconds
		*/
		void FixedUpdate(float p_deltaTime);

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
		bool DestroyEntity(Entity& p_entity);

		void CollectGarbages();

		/**
		* Return the first gameobject identified by the given name, or nullptr on fail
		* @paam p_name
		*/
		Entity* GetEntityByName(const std::string& p_name) const;

		/**
		* Return the first gameobject identified by the given tag, or nullptr on fail
		* @param p_tag
		*/
		Moon::Entity* GetEntityByTag(const std::string& p_tag) const;

		/**
		* Return the gameobject identified by the given ID (Returns nullptr on fail)
		* @param p_id
		*/
		Entity* GetEntityByID(int64_t p_id) const;

		/**
		* 
		*/
		CameraComponent* FindMainCamera() const;

		/**
		* Callback method called everytime a component is added on an entity of the scene
		* @param p_component
		*/
		void OnComponentAdded(AComponent& p_component);

		/**
		* Callback method called everytime a component is removed on an entity of the scene
		* @param p_component
		*/
		void OnComponentRemoved(AComponent& p_component);

		/**
		* Return a refrence on the gameobject map
		*/
		std::vector<std::unique_ptr<Entity>>& GetAllEntities();
		const std::vector<std::unique_ptr<Entity>>& GetAllEntities() const;

		/**
		* Return the fast access components data structure
		*/
		const FastAccessComponents& GetFastAccessComponents() const;

		Moon::Entity& InstantiatePrefab(const std::string& prefabPath);
		Moon::Entity& InstantiatePrefabChild(Moon::Entity& parent, const json& childJson);

		/* =================== */
		std::string FullPath;

		bool EntityExists(const std::string& name);

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
		std::vector<std::unique_ptr<Entity>> m_entities;
		std::unordered_map<uint32_t, Entity*> m_EntityIDMap;
		std::unordered_map<std::string, Entity*> m_EntityNameMap;

		FastAccessComponents m_fastAccessComponents;
	};
}