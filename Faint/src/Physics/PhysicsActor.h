#pragma once

#include <memory>
#include <any>
#include <string>

#include <Scene/Entity.h>
#include <Math/Transform.h>
#include <Event/Event.h>

namespace physx {
	class PxRigidActor;
	class PxShape;
	class PxMaterial;
}

namespace Moon::Physics {
	
	// Forward declaration
	class PhysicsEngine;

	enum CollisionGroup {
		NO_COLLISION = 0,
		CHARACTER_CONTROLLER,
		DYNAMIC_OBJECT,
		STATIC_OBJECT,
		TRIGGER_OBJECT
	};

	enum RaycastGroup {
		RAYCAST_DISABLED = 0,
		RAYCAST_ENABLED
	};

	struct PhysicsFilterData {
		RaycastGroup raycastGroup = RaycastGroup::RAYCAST_DISABLED;
		CollisionGroup collisionGroup = CollisionGroup::NO_COLLISION;
		CollisionGroup collidesWith = CollisionGroup::NO_COLLISION;
	};

	enum class ForceMode {
		Force,
		Impulse,
		VelocityChange,
		Acceleration
	};

	class PhysicsActor {
	public:
		friend class PhysicsEngine;

		PhysicsActor();
		PhysicsActor(Transform& p_transform);
		~PhysicsActor();

		virtual void CreateActor() = 0;
		
		void DestroyActor();

		// Body type
		//void SetBodyType(BodyType type);
		//BodyType GetBodyType() const;

		// Basic properties
		void SetTrigger(bool trigger);
		bool IsTrigger() const;

		void SetKinematic(bool kinematic);
		bool IsKinematic() const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const;

		// Damping
		void SetLinearDamping(float damping);
		float GetLinearDamping() const;

		void SetAngularDamping(float damping);
		float GetAngularDamping() const;

		// Gravity
		void SetGravityScale(float scale);
		float GetGravityScale() const;

		// Mass
		void SetMass(float mass);
		float GetMass() const;

		// Motion constraints
		void SetLinearLockFlags(uint8_t flags);
		uint8_t GetLinearLockFlags() const;
		
		// Forces and velocities
		void AddForce(const glm::vec3& force, bool autoWake = true);
		void ClearForces();

		// Velocities
		void SetLinearVelocity(const glm::vec3& velocity);
		glm::vec3 GetLinearVelocity() const;

		void SetAngularVelocity(const glm::vec3& velocity);
		glm::vec3 GetAngularVelocity() const;

		// Kinematic movement
		void SetKinematicTarget(const glm::vec3& position, const glm::quat& rotation);

		// Sleep control
		void WakeUp();
		void PutToSleep();
		bool IsSleeping() const;

		// Physics material
		void SetBounciness(float bounciness);
		float GetBounciness() const;

		void SetStaticFriction(float friction);
		float GetStaticFriction() const;

		void SetDynamicFriction(float friction);
		float GetDynamicFriction() const;

		// Collision filtering
		void SetCollisionGroup(CollisionGroup groups);
		CollisionGroup GetCollisionGroup() const;

		void SetCollidesWith(CollisionGroup groups);
		CollisionGroup GetCollidesWith() const;

		// Utility methods
		void Consider();
		void Unconsider();

		physx::PxRigidActor* GetActor();
		const physx::PxRigidActor* GetActor() const;

		// User data managment
		template<typename T>
		void SetUserData(T&& value) {
			m_userData = std::forward<T>(value);
		}
		template<typename T>
		T GetUserData() const { return std::any_cast<T>(m_userData); }

		std::any GetInlineUserData() const { return m_userData; }

	protected:
		void Init();
		void RecreateBody();
		virtual void UpdatePhysicsMaterial();
		virtual void ApplyDamping();
		virtual void ApplyConstraints();
		
	public:
		std::string m_name;
		Entity* m_entity = nullptr;

		Event<PhysicsActor&> CollisionStartEvent;
		Event<PhysicsActor&> CollisionStayEvent;
		Event<PhysicsActor&> CollisionStopEvent;
		Event<PhysicsActor&> TriggerEnterEvent;
		Event<PhysicsActor&> TriggerStayEvent;
		Event<PhysicsActor&> TriggerExitEvent;
		Event<> SleepEvent;
		Event<> WakeEvent;

	private:
		/* Internal */
		virtual void CreateShape() = 0;
		void DestroyBody();
		virtual void UpdatePhysxTransform() = 0;
		virtual void UpdateTransform() = 0;

	protected:
		/* PhysX stuff */
		physx::PxRigidActor* m_actor = nullptr;
		physx::PxShape* m_shape = nullptr;
		physx::PxMaterial* m_material = nullptr;

		Transform* m_transform = nullptr;
		//BodyType m_bodyType = BodyType::Dynamic;

		// Advanced properties
		float m_mass = 0.0f;
		float m_linearDamping = 0.0f;
		float m_angularDamping = 0.0f;
		float m_gravityScale = 1.0f;
		
		// Motion constraints
		uint8_t m_linearLockFlags = 0;
		uint8_t m_angularLockFlags = 0;

		// Velocity limits
		float m_maxLinearVelocity = 500.0f;
		float m_maxAngularVelocity = 500.0f;

		// Physics material
		float m_bounciness = 0.0f;
		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		uint8_t m_frictionCombineMode = 0;
		uint8_t m_restitutionCombineMode = 0;

		// Collision filtering
		PhysicsFilterData m_filterData;
	private:
		/* Transform stuff*/
		bool m_internalTransform;

		/* Settings */
		bool m_kinematic = false;
		bool m_isTrigger = false;
		bool m_enabled = true;
		bool m_considered = false;

		/* Other */
		std::any m_userData;
		static Event<PhysicsActor&> CreatedEvent;
		static Event<PhysicsActor&> DestroyedEvent;
		static Event<physx::PxRigidActor&> ConsiderEvent;
		static Event<physx::PxRigidActor&> UnconsiderEvent;
	};
}