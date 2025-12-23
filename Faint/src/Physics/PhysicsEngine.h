#pragma once
#include <Physics/PhysicsActor.h>
#include <Physics/CharacterController.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <mutex>

// TODO: ?
#pragma warning(push, 0)
#include "PxPhysicsAPI.h"
#pragma warning(pop)
//namespace physx {
//	class PxPhysics;
//	class PxScene;
//	class PxFoundation;
//	class PxDefaultCpuDispatcher;
//	class PxPvd;
//	class PxMaterial;
//	class PxControllerManager;
//	class PxActor;
//	class PxConstraintInfo;
//	typedef uint32_t PxU32;
//}

namespace Moon::Physics {

	struct PairHash {
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& p) const noexcept {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			// ترکیب دو hash (روش ساده XOR + shift)
			return h1 ^ (h2 << 1);
		}
	};

	struct PhysXRayResult {
		PhysicsActor* actor = nullptr;
		std::string name;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 direction = glm::vec3(0);
		bool found = false;
		float distance = 0.0f;
	};

	class PhysicsEngine {
	private:
		class MyControllerHitReport : public physx::PxUserControllerHitReport {
		public:
			void onShapeHit(const physx::PxControllerShapeHit& hit) override;
			void onControllerHit(const physx::PxControllersHit& hit) override;
			void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
		};
	public:
		PhysicsEngine();
		~PhysicsEngine();

		void Update(float p_deltaTime, bool simulate = true);

		void QueuePhysicsChange(const std::function<void()>& change);

		std::vector<PhysXRayResult> CastPhysXRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, uint32_t collisionFlags, bool cullBackFacing = false);
		PhysXRayResult CastPhysXRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, uint32_t collisionFlags, bool cullBackFacing = false, bool firstOnly = true);

		physx::PxPhysics* PxPhysics() const;
		physx::PxScene* PxScene() const;
		physx::PxMaterial* GetDefaultMaterial() const;
		MyControllerHitReport* GetControllerHitReport() const;
		physx::PxControllerManager* GetCharacterControllerManager() const;

		std::vector<std::reference_wrapper<PhysicsActor>> GetPhysicsActors() const;

	private:
		void Initialize();
		void ListenToPhysicsActors();

		void Consider(PhysicsActor& p_toConsider);
		void UnConsider(PhysicsActor& p_toUnconsider);

		void Consider(physx::PxRigidActor& actor);
		void UnConsider(physx::PxRigidActor& actor);

		void ResetCollisionEvents();
		void CheckCollisionStopEvents();

		class ContactReportCallback : public physx::PxSimulationEventCallback {
		public:
			void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
			void onWake(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
			void onSleep(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
			void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
			void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) {}

			void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* /*pairs*/, physx::PxU32 /*nbPairs*/);
		};

	private:
		float m_accumulator = 0.0f;
		const float m_fixedDeltaTime = 1.0f / 60.0f;
		std::mutex m_changeMutex;
		std::vector<std::function<void()>> m_pendingChanges;

		physx::PxPhysics* m_physics = nullptr;
		physx::PxScene* m_scene = nullptr;
		physx::PxCookingParams* m_cooking = nullptr;
		physx::PxFoundation* m_foundation = nullptr;
		std::unique_ptr<ContactReportCallback> m_contactCallback;
		std::unique_ptr<MyControllerHitReport> m_characterControllerCallback;

		physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
		physx::PxPvd* m_pvd = nullptr;
		physx::PxMaterial* m_defaultMaterial = nullptr;
		physx::PxControllerManager* m_characterControllerManager = nullptr;

		static std::map<std::pair<PhysicsActor*, PhysicsActor*>, bool> m_collisionEvents;
		static std::map<std::pair<Moon::Physics::PhysicsActor*, PhysicsActor*>, bool> m_cctContacts;
		static std::unordered_set<std::pair<Moon::Physics::PhysicsActor*, PhysicsActor*>, PairHash> m_currentFrameContacts;
		std::vector<std::reference_wrapper<PhysicsActor>> m_physicsActors;
	};
}