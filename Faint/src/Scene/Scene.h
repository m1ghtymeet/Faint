#pragma once

#include "Core/Time.h"
#include "Common/UUID.h"
#include "Debug/MulticastDelegate.h"
#include "Renderer/EditorCamera.h"
#include "Renderer/Types/Framebuffer2.h"
#include "Scene/Environment/Environment.h"

#include "Scene/Systems/PhysicsSystem.h"
#include "Scene/Systems/TransformSystem.h"
#include "Scene/Systems/ScriptingSystem.h"

#include <entt.hpp>

namespace Faint {

	class Entity;
	class PhysicsSystem;

	DECLARE_MULTICAST_DELEGATE(PreInitializeDelegate);
	DECLARE_MULTICAST_DELEGATE(PostInitializeDelegate);

	enum SelectedType {
		NONE = 0,
		_CAMERA,
		_LIGHT
	};

	class Scene : public ISerializable {
	private:

		std::vector<Ref<System>> m_systems;

		Ref<Environment> environment;
	public:
		Ref<ScriptingSystem> m_ScriptingSystem;

		Scene();
		~Scene();

		std::string FullPath;

		bool OnInit();
		void OnExit();
		void Update(Time time);
		void EditorUpdate(Time time);
		void FixedUpdate(Time time);

		void Draw();
		void Draw(const Matrix4& projection, const Matrix4& view);

		static Ref<Scene> Copy(Ref<Scene> other);
		static Ref<Scene> Copy(const std::string& path);
		static Ref<Scene> Copy();
		static Ref<Scene> New();
		static Ref<Scene> Save();

		Entity CreateEntity(std::string name = "undefined");
		Entity CreateEntityWithUUID(UUID uuid, std::string name = "undefined");
		void DestroyEntity(Entity entity);
		bool EntityExists(const std::string& name);

		std::vector<Entity> GetAllEntities();
		Entity GetEntity(const std::string& name);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByID(int id);

		bool EntityIsParent(Entity entity, Entity parent);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateEditor(Time time, EditorCamera& camera, Framebuffer2& frameBuffer);
		void OnUpdateSimulation(Time time, EditorCamera& camera, Framebuffer2& frameBuffer);
		void OnUpdateRuntime(Time time, Framebuffer2& frameBuffer);
		void OnViewportResize(float width, float height);

		Entity DuplicateEntity(Entity entity);

		Ref<Camera> GetCurrentCamera();

		entt::registry& Reg() { return _registry; }

		Entity GetPrimaryCameraEntity();
		Ref<Environment> GetEnvironment() { return environment; }

		//Ref<PhysicsSystem> GetPhysicsSystem() const { return physicsSystem; }

		Ref<EditorCamera> m_EditorCamera;
		SelectedType selectedType;

		json Serialize() override;
		bool Deserialize(const json& j) override;
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry _registry;
		float _viewportWidth = 1280, _viewportHeight = 720;

		float fixedUpdateRate = 1.0f / 90.0f;
		float fixedUpdateDifference = 0;

		std::unordered_map<uint32_t, Entity> m_EntityIDMap;
		std::unordered_map<std::string, Entity> m_EntityNameMap;

		PreInitializeDelegate preInitializeDelegate;
		PostInitializeDelegate postInitializeDelegate;

		friend Entity;
		friend class SceneSerializer;
		friend class SceneHierarchy;
	};
}