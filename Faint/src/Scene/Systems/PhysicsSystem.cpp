#include "PhysicsSystem.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Physics/Physics.h"
#include "Engine.h"
#include "Scene/Components.h"

namespace Faint {
	PhysicsSystem::PhysicsSystem(Scene* scene) {
		m_scene = scene;
	}

	bool PhysicsSystem::Init() {
		return true;
	}

	void PhysicsSystem::Update(Time ts) {
		//if (!Engine::IsPlayMode()) {
		//	SyncTransformsOnly();
		//}
		//
		//auto view = m_scene->Reg().view<TransformComponent, RigidBodyComponent>();
		//for (auto e : view) {
		//	auto [transform, rigidBody] = view.get<TransformComponent, RigidBodyComponent>(e);
		//	Entity entity = *m_scene->GetEntityByID((int)e);
		//
		//	if (!rigidBody.actor && entity.GetComponent<BoxColliderComponent>()) {
		//		BoxColliderComponent* box = entity.GetComponent<BoxColliderComponent>();
		//		PxShape* shape = Physics::CreateBoxShape(box->halfExtents.x, box->halfExtents.y, box->halfExtents.z);
		//
		//		PxTransform trans = PxTransform(
		//			Physics::GlmVec3toPxVec3(transform.GetLocalPosition()),
		//			Physics::GlmQuatToPxQuat(transform.GetLocalRotation())
		//		);
		//
		//		PhysicsFilterData filter;
		//		filter.raycastGroup   = RAYCAST_DISABLED;
		//		filter.collisionGroup = rigidBody.mass > 0.0f ? DYNAMIC_OBJECT : STATIC_OBJECT;
		//		filter.collidesWith   = rigidBody.mass > 0.0f ? STATIC_OBJECT : DYNAMIC_OBJECT;
		//
		//		if (rigidBody.mass > 0.0f) {
		//			if (PxRigidStatic* stat = rigidBody.GetStaticBody())
		//				Physics::Destroy(stat);
		//			rigidBody.actor = Physics::CreateRigidDynamic(transform.GetTransform(), filter, shape);
		//			PxRigidDynamic* dynamic = rigidBody.GetDynamicBody();
		//			if (dynamic)
		//				PxRigidBodyExt::updateMassAndInertia(*dynamic, rigidBody.mass);
		//		}
		//		else {
		//			if (PxRigidDynamic* dynamic = rigidBody.GetDynamicBody())
		//				Physics::Destroy(dynamic);
		//			PxFilterData filterData;
		//			filterData.word0 = (PxU32)filter.raycastGroup;
		//			filterData.word1 = (PxU32)filter.collisionGroup;
		//			filterData.word2 = (PxU32)filter.collidesWith;
		//			shape->setQueryFilterData(filterData);
		//			shape->setSimulationFilterData(filterData);
		//			rigidBody.actor = Physics::GetPxPhysics()->createRigidStatic(trans);
		//			rigidBody.actor->attachShape(*shape);
		//			Physics::GetPxScene()->addActor(*rigidBody.actor);
		//		}
		//	}
		//}
		//
		//Engine::GetCurrentScene()->Reg().view<RigidBodyComponent, TransformComponent>().each([](auto entity, RigidBodyComponent& rb, TransformComponent& transform) {
		//	if (auto* dynamic = rb.GetDynamicBody()) {
		//		PxTransform pxTransform = dynamic->getGlobalPose();
		//		transform.SetLocalPosition(Physics::PxVec3toGlmVec3(pxTransform.p));
		//		transform.SetLocalRotation(glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z));
		//	}
		//	});
	}

	void PhysicsSystem::FixedUpdate(Time ts) {
		if (Engine::IsPlayMode()) {
			Physics::StepPhysics(ts);
		}
		else {
			Physics::GetPxScene()->simulate(1e-6f);
			Physics::GetPxScene()->fetchResults(true);
		}
	}

	void PhysicsSystem::Exit() {
		auto view = m_scene->Reg().view<RigidBodyComponent>();
		for (auto e : view) {
			RigidBodyComponent& rb = view.get<RigidBodyComponent>(e);
			if (auto* dynamic = rb.GetDynamicBody()) {
				dynamic->setLinearVelocity(PxVec3(0.0f));
				dynamic->setAngularVelocity(PxVec3(0.0f));
			}
		}
	}

	void PhysicsSystem::SyncTransformsOnly() {
		//auto view = m_scene->Reg().view<TransformComponent, BoxColliderComponent, RigidBodyComponent>();
		//for (auto e : view) {
		//	auto [tr, box, rb] = view.get<TransformComponent, BoxColliderComponent, RigidBodyComponent>(e);
		//	if (!rb.actor) continue;
		//
		//	// Translate RigidBody
		//	PxTransform trans(
		//		Physics::GlmVec3toPxVec3(tr.GetLocalPosition()),
		//		Physics::GlmQuatToPxQuat(tr.GetLocalRotation())
		//	);
		//	rb.actor->setGlobalPose(trans);
		//
		//	// Update geometry
		//	PxShape* shape = nullptr;
		//	PxU32 shapeCount = rb.actor->getNbShapes();
		//	if (shapeCount > 0) {
		//		rb.actor->getShapes(&shape, 1);
		//		PxBoxGeometry boxGeom;
		//		boxGeom.halfExtents = Physics::GlmVec3toPxVec3(box.halfExtents);
		//		shape->setGeometry(boxGeom);
		//	}
		//}
		//
		//auto charView = m_scene->Reg().view<TransformComponent, CharacterControllerComponent>();
		//for (auto e : charView) {
		//	auto [tr, character] = charView.get<TransformComponent, CharacterControllerComponent>(e);
		//	glm::vec3 position = tr.GetLocalPosition();
		//	character.controller->setPosition(PxExtendedVec3(position.x, position.y, position.z));
		//}
	}
}