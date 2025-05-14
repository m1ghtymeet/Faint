#include "SceneNetAPI.h"

#include "Engine.h"
#include "Scripting/ScriptingEngineNet.h"
#include "Physics/Physics.h"

#include "Scene/Components.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/ParentComponent.h"
#include "Scene/Components/PrefabComponent.h"

#include <Coral/Array.hpp>

namespace Faint {

	uint32_t GetEntity(Coral::String entityName)
	{
		auto scene = Engine::GetCurrentScene();
		if (!scene->EntityExists(entityName))
			return UINT32_MAX;

		return scene->GetEntity(entityName).GetHandle();
	}

	Coral::ManagedObject GetEntityScriptFromName(Coral::String entityName)
	{
		auto scene = Engine::GetCurrentScene();
		if (!scene->EntityExists(entityName))
			return Coral::ManagedObject(); // Error code: entity not found.

		Entity entity = scene->GetEntity(entityName);

		auto& scriptingEngine = ScriptingEngineNet::Get();
		if (scriptingEngine.HasEntityScriptInstance(entity)) {
			auto instance = scriptingEngine.GetEntityScript(entity);
		}
	}

	enum ComponentTypes
	{
		Unknown = -1,
		PARENT = 0,
		NAME,
		PREFAB,
		TRANSFORM,
		LIGHT,
		CAMERA,
		AUDIO_EMITTER,
		MODEL,
		SKINNED_MODEL,
		BONE,
		RIGIDBODY,
		BOX_COLLIDER,
		SPHERE_COLLIDER,
		CAPSULE_COLLIDER,
		CYLINDER_COLLIDER,
		MESH_COLLIDER,
		CHARACTER_CONTROLLER,
		PARTICLE_EMITTER,
		QUAKE_MAP,
		BSP_BRUSH,
		SPRITE,
		NAVMESH
	};

	bool EntityHasComponent(int id, int componentType)
	{
		Entity entity = { (entt::entity)(id), Engine::GetCurrentScene().get() };
		if (!entity.IsValid())
			return false;

		switch (static_cast<ComponentTypes>(componentType))
		{
		case PARENT:				return entity.HasComponent<ParentComponent>();
		case NAME:					return entity.HasComponent<NameComponent>();
		case PREFAB:				return entity.HasComponent<PrefabComponent>();
		case TRANSFORM:				return entity.HasComponent<TransformComponent>();
		case LIGHT:					return entity.HasComponent<LightComponent>();
		case CAMERA:				return entity.HasComponent<CameraComponent>();
		case AUDIO_EMITTER:			return entity.HasComponent<AudioComponent>();
		case MODEL:					return entity.HasComponent<MeshRendererComponent>();
		//case SKINNED_MODEL:			return entity.HasComponent<SkinnedModelComponent>();
		//case BONE:					return entity.HasComponent<BoneComponent>();
		case BOX_COLLIDER:			return entity.HasComponent<BoxColliderComponent>();
		case SPHERE_COLLIDER:		return entity.HasComponent<SphereColliderComponent>();
		//case CAPSULE_COLLIDER:		return entity.HasComponent<CapsuleColliderComponent>();
		//case CYLINDER_COLLIDER:		return entity.HasComponent<CylinderColliderComponent>();
		//case MESH_COLLIDER:			return entity.HasComponent<MeshColliderComponent>();
		case CHARACTER_CONTROLLER:	return entity.HasComponent<CharacterControllerComponent>();
		//case PARTICLE_EMITTER:		return entity.HasComponent<ParticleEmitterComponent>();
		//case QUAKE_MAP:				return entity.HasComponent<QuakeMapComponent>();
		case BSP_BRUSH:				return entity.HasComponent<BSPBrushComponent>();
		case SPRITE:				return entity.HasComponent<SpriteComponent>();
		//case NAVMESH:				return entity.HasComponent<NavMeshVolumeComponent>();
		default:
			return false;
		}
	}

	void EntityAddComponent(int id, int componentType) {

		Entity entity = { (entt::entity)(id), Engine::GetCurrentScene().get() };

		if (!entity.IsValid())
			return;

		switch (static_cast<ComponentTypes>(componentType))
		{
		case PARENT:				entity.AddComponent<ParentComponent>(); break;
		case NAME:					entity.AddComponent<NameComponent>(); break;
		case PREFAB:				entity.AddComponent<PrefabComponent>(); break;
		case TRANSFORM:				entity.AddComponent<TransformComponent>(); break;
		case LIGHT:					entity.AddComponent<LightComponent>(); break;
		case CAMERA:				entity.AddComponent<CameraComponent>(); break;
		case AUDIO_EMITTER:			entity.AddComponent<AudioComponent>(); break;
		case MODEL:					entity.AddComponent<MeshRendererComponent>(); break;
		//case SKINNED_MODEL:			entity.AddComponent<SkinnedModelComponent>(); break;
		//case BONE:					entity.AddComponent<BoneComponent>(); break;
		case BOX_COLLIDER:			entity.AddComponent<BoxColliderComponent>(); break;
		case SPHERE_COLLIDER:		entity.AddComponent<SphereColliderComponent>(); break;
		//case CAPSULE_COLLIDER:		entity.AddComponent<CapsuleColliderComponent>(); break;
		//case CYLINDER_COLLIDER:		entity.AddComponent<CylinderColliderComponent>(); break;
		//case MESH_COLLIDER:			entity.AddComponent<MeshColliderComponent>(); break;
		case CHARACTER_CONTROLLER:	entity.AddComponent<CharacterControllerComponent>(); break;
//		case PARTICLE_EMITTER:		entity.AddComponent<ParticleEmitterComponent>(); break;
//		case QUAKE_MAP:				entity.AddComponent<QuakeMapComponent>(); break;
		case BSP_BRUSH:				entity.AddComponent<BSPBrushComponent>(); break;
		case SPRITE:				entity.AddComponent<SpriteComponent>(); break;
		//case NAVMESH:				entity.AddComponent<NavMeshVolumeComponent>(); break;
		}
	}

	int EntityGetEntity(int handle, Coral::String input) {

		Ref<Scene> scene = Engine::GetCurrentScene();
		if (String::BeginsWith(input, "/")) {
			return 1;
		}

		Entity entity = { (entt::entity)handle, scene.get() };
		return 1; // SOOON...
	}

	Coral::String EntityGetName(int handle)
	{
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		if (!entity.IsValid())
			return Coral::String();

		return Coral::String::New(entity.GetComponent<NameComponent>().name);
	}

	void EntitySetName(int handle, Coral::String newName) {

	}

	bool EntityIsValid(int handle) {
		Entity entity = { (entt::entity)(handle), Engine::GetCurrentScene().get() };
		return entity.IsValid();
	}

	void TransformSetPosition(int entityID, float x, float y, float z) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalPosition(Vec3(x, y, z));

			if (entity.HasComponent<CharacterControllerComponent>())
			{
				//PhysicsManager::Get().SetCharacterControllerPosition(entity, { x, y, z });
			}
		}
	}

	Coral::Array<float> TransformGetPosition(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			const auto& position = component.GetLocalPosition();
			Coral::Array<float> result = Coral::Array<float>::New({position.x, position.y, position.z});
			return result;
		}
	}

	void TransformSetGlobalRotation(int entityID, float x, float y, float z) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			component.SetGlobalRotation(Vec3(x, y, z));
		}
	}

	Coral::Array<float> TransformGetGlobalRotation(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			const auto& rotation = component.GetGlobalRotation();
			Coral::Array<float> result = Coral::Array<float>::New({ rotation.x, rotation.y, rotation.z });
			return result;
		}
	}

	void TransformSetRotation(int entityID, float x, float y, float z) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalRotation(Vec3(x, y, z));
		}
	}

	Coral::Array<float> TransformGetRotation(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			auto& component = entity.GetComponent<TransformComponent>();
			const auto& rotation = component.GetLocalRotation();
			Coral::Array<float> result = Coral::Array<float>::New({ rotation.x, rotation.y, rotation.z });
			return result;
		}
	}

	void TransformRotate(int entityId, float x, float y, float z) {

		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<TransformComponent>()) {

			Quat quat = Math::QuatFromEuler(x, y, z);
			auto& component = entity.GetComponent<TransformComponent>();
			component.SetLocalRotation(quat);
		}
	}

	/* ^^^ Camera ^^^ */
	float CameraFieldOfView(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>()) {

			auto& component = entity.GetComponent<CameraComponent>();
			return component.camera->m_fieldOfView;
		}
		return -1.0f;
	}

	Coral::Array<float> CameraGetUp(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>()) {

			auto& component = entity.GetComponent<CameraComponent>();
			const auto& up = component.camera->GetUp();
			Coral::Array<float> result = Coral::Array<float>::New({ up.x, up.y, up.z });
			return result;
		}
	}

	Coral::Array<float> CameraGetRight(int entityID) {

		Entity entity = { (entt::entity)(entityID), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CameraComponent>()) {

			auto& component = entity.GetComponent<CameraComponent>();
			const auto& right = component.camera->GetRight();
			Coral::Array<float> result = Coral::Array<float>::New({ right.x, right.y, right.z });
			return result;
		}
	}

	void MoveAndSlide(int entityId, float vx, float vy, float vz)
	{
		Entity entity = { (entt::entity)(entityId), Engine::GetCurrentScene().get() };

		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& component = entity.GetComponent<CharacterControllerComponent>();

			if (std::isnan(vx) || std::isnan(vy) || std::isnan(vz))
			{
				return; // Log message here? invalid input
			}

			//component.GetCharacterController()->MoveAndSlide({ vx, vy, vz });
		}
	}

	bool IsOnGround(int entityId)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<CharacterControllerComponent>())
		{
			auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			//return PhysicsManager::Get().GetWorld()->IsCharacterGrounded(entity);
		}

		return false;
	}

	void PlayAudioByPath(int entityId, std::string path, float volume)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());
	
		if (entity.IsValid() && entity.HasComponent<AudioComponent>())
		{
			auto& audio = entity.GetComponent<AudioComponent>();
			audio.IsPlaying = true;
			audio.Volume = volume;
			audio.FilePath = path;
		}
	}

	void PlayAudio(int entityId, float volume)
	{
		Entity entity = Entity((entt::entity)(entityId), Engine::GetCurrentScene().get());

		if (entity.IsValid() && entity.HasComponent<AudioComponent>())
		{
			auto& audio = entity.GetComponent<AudioComponent>();
			audio.IsPlaying = true;
			audio.Volume = volume;
		}
	}

	void SceneNetAPI::RegisterMethods()
	{
		// Entity
		RegisterMethod("Entity.EntityHasComponentIcall", &EntityHasComponent);
		RegisterMethod("Entity.EntityAddComponentIcall", &EntityAddComponent);
		RegisterMethod("Entity.EntityGetEntityIcall", &EntityGetEntity);
		RegisterMethod("Entity.EntityGetNameIcall", &EntityGetName);
		RegisterMethod("Entity.EntitySetNameIcall", &EntitySetName);
		RegisterMethod("Entity.EntityIsValidIcall", &EntityIsValid);

		// Scene
		RegisterMethod("Scene.GetEntityIcall", &GetEntity);
		RegisterMethod("Scene.GetEntityScriptFromNameIcall", &GetEntityScriptFromName);

		// ^^^ Components ^^^
		// Transform 
		RegisterMethod("TransformComponent.SetPositionIcall", &TransformSetPosition);
		RegisterMethod("TransformComponent.GetRotationIcall", &TransformGetRotation);
		RegisterMethod("TransformComponent.GetGlobalRotationIcall", &TransformGetGlobalRotation);
		RegisterMethod("TransformComponent.SetGlobalRotationIcall", &TransformSetGlobalRotation);
		RegisterMethod("TransformComponent.SetRotationIcall", &TransformSetRotation);
		RegisterMethod("TransformComponent.RotateIcall", &TransformRotate);

		// Camera 
		RegisterMethod("CameraComponent.GetFieldOfViewIcall", &CameraFieldOfView);
		RegisterMethod("CameraComponent.GetCameraUpIcall", &CameraGetUp);
		RegisterMethod("CameraComponent.GetCameraRightIcall", &CameraGetRight);
		
		// Character Controller 
		RegisterMethod("CharacterControllerComponent.MoveAndSlideIcall", &MoveAndSlide);
		RegisterMethod("CharacterControllerComponent.IsOnGroundedIcall", &IsOnGround);

		// Audio
		RegisterMethod("AudioComponent.PlayAudioByPathIcall", &PlayAudioByPath);
		RegisterMethod("AudioComponent.PlayAudioIcall", &PlayAudio);
	}
}
