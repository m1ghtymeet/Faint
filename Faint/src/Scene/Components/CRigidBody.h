#pragma once

#include "AssetManagment/Serializable.h"
#include "CCollider.h"
#include <memory>

namespace Moon {
	namespace Physics {
		class PhysicsActor;
		enum class BodyType;
	}
	class RigidBodyComponent : public ColliderComponent {
	public:
		RigidBodyComponent(Entity& p_owner);
		~RigidBodyComponent();

		std::string GetName() override;

		// Basic properties
		void SetKinematic(bool kinematic);
		bool IsKinematic() const;

		void SetTrigger(bool trigger);
		bool IsTrigger() const;

		void SetMass(float mass);
		float GetMass() const;

		// Advanced properties
		void SetLinearDrag(float drag);
		float GetLinearDrag() const;

		void SetAngularDrag(float drag);
		float GetAngularDrag() const;

		void SetGravityScale(float scale);
		float GetGravityScale() const;

		// Motion constraints
		void SetFreezePosition(const glm::bvec3& freeze);
		glm::bvec3 GetFreezePosition() const;

		void SetFreezeRotation(const glm::bvec3& freeze);
		glm::bvec3 GetFreezeRotation() const;

		// Sleep properties
		void SetEnableSleep(bool enable);
		bool GetEnableSleep() const;

		void SetSleepThreshold(float threshold);
		bool GetSleepThreshold() const;

		// Physics material properties
		void SetBounciness(float bounciness);
		float GetBounciness() const;

		void SetFriction(float friction);
		float GetFriction() const;

		void SetStaticFriction(float friction);
		float GetStaticFriction() const;

		void SetDynamicFriction(float friction);
		float GetDynamicFriction() const;

		// Forces and velocities
		void AddForce(const glm::vec3& force);
		void AddTorque(const glm::vec3& torque);
		void SetVelocity(const glm::vec3& velocity);
		glm::vec3 GetVelocity() const;
		void SetAngularVelocity(const glm::vec3& velocity);
		glm::vec3 GetAngularVelocity() const;

		void MovePosition(const glm::vec3& position);
		void MoveRotation(const glm::quat& rotation);

		// Utility methods
		void WakeUp();
		void PutToSleep();
		bool IsSleeping() const;

		/**
		* Serialize the component
		*/
		json Serialize();

		/**
		* Deserialize the component
		* @param j
		*/
		void Deserialize(const json& j);

	public:
		Event<> SleepEvent;
		Event<> WakeEvent;
	protected:
		template <typename T> T& GetPhysicsActorAs() const {
			return *static_cast<T*>(m_physicsActor.get());
		}
		void BindListener();
		void ApplyConstraints();
		void UpdatePhysicsMaterial();

	private:
		virtual void OnEnable() override;
		virtual void OnDisable() override;
		virtual void Init() {}

	protected:
		std::unique_ptr<Physics::PhysicsActor> m_physicsActor;

		// Basic properties
		float m_mass = 1.0f;

		// Advanced properties
		float m_linearDrag = 0.0f;
		float m_angularDrag = 0.05f;
		float m_gravityScale = 1.0f;

		// Motion constraints
		glm::bvec3 m_freezePosition = { 0, 0, 0 };
		glm::bvec3 m_freezeRotation = { 0, 0, 0 };

		// Sleep properties
		bool m_enableSleep = true;
		float m_sleepThreshold = 0.1f;

		// Physics material
		float m_bounciness = 0.0f;
		float m_friction = 0.5f;
		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
	};
}