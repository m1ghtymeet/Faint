#include "PhysicsSystem.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include <Scene/Components/ParentComponent.h>
#include "Physics/Physics.h"
#include "Engine.h"

namespace Faint
{
	PhysicsSystem::PhysicsSystem(Scene* scene)
	{
		m_scene = scene;
	}

	bool PhysicsSystem::Init()
	{
		//Logger::Log("Initializing the physic system");
		HZ_CORE_TRACE("Initializing the physic system");

		//Physics::Init();

		// We need to initialize shapes first, then bodies...
		//InitializeShapes();
		//
		//InitializeRigidbodies();
		//InitializeCharacterControllers();
		//InitializeQuakeMap();

		HZ_CORE_TRACE("Physic system initialized successfully");
		//Logger::Log("Physic system initialized successfully");
		return true;
	}

	void PhysicsSystem::Update(Time ts)
	{
		if (!Engine::IsPlayMode()) {
			return;
		}
		
		//InitializeShapes();
		//InitializeRigidbodies();
		//InitializeCharacterControllers();
		//
		//ApplyForces();
		//
		//PhysicsManager::Get().Step(ts);
		//
		//auto brushes = m_scene->Reg().view<TransformComponent, BSPBrushComponent>();
		//for (auto e : brushes)
		//{
		//	auto [transform, brush] = brushes.get<TransformComponent, BSPBrushComponent>(e);
		//
		//	if (!brush.IsFunc)
		//		continue;
		//
		//	brush.Targets.clear();
		//	auto targetnameView = m_scene->Reg().view<TransformComponent, NameComponent>();
		//	for (auto e2 : targetnameView)
		//	{
		//		auto [ttransform, name] = targetnameView.get<TransformComponent, NameComponent>(e2);
		//
		//		if (name.name == brush.target)
		//		{
		//			brush.Targets.push_back(Entity{ e2, m_scene });
		//		}
		//	}
		//}
	}

	void PhysicsSystem::FixedUpdate(Time ts)
	{
		if (!Engine::IsPlayMode())
			return;

		auto view = m_scene->Reg().view<TransformComponent, BoxColliderComponent, RigidBodyComponent>();
		for (auto e : view)
		{
			auto [transform, box, rigidBody] = view.get<TransformComponent, BoxColliderComponent, RigidBodyComponent>(e);
			if (!rigidBody.dynamicActor) {
				PxShape* shape = Physics::CreateBoxShape(box.halfExtents.x, box.halfExtents.y, box.halfExtents.z);

				//PhysicsFilterData filterData;
				//filterData.raycastGroup = RAYCAST_DISABLED;
				//filterData.collisionGroup = CollisionGroup::GENERIC_BOUNCEABLE;
				//filterData.collidesWith = (CollisionGroup)(ENVIROMENT_OBSTACLE);
				//rigidBody.dynamicActor = Physics::CreateRigidDynamic(transform.GetTransform(), filterData, shape);
				PxQuat quat = Physics::GlmQuatToPxQuat(transform.GetLocalRotation());
				PxTransform trans = PxTransform(PxVec3(transform.GetLocalPosition().x, transform.GetLocalPosition().y, transform.GetLocalPosition().z), quat);
				rigidBody.dynamicActor = Physics::GetPxPhysics()->createRigidDynamic(trans);
				rigidBody.dynamicActor->attachShape(*shape);

				PxRigidBodyExt::updateMassAndInertia(*rigidBody.dynamicActor, box.density);
				Physics::GetPxScene()->addActor(*rigidBody.dynamicActor);
			}
		}

		Physics::StepPhysics(ts);
		Physics::SubmitDebugLinesToRenderer(DebugRenderMode::PHYSX_ALL);

		Engine::GetCurrentScene()->Reg().view<RigidBodyComponent, TransformComponent>().each([](auto entity, RigidBodyComponent& rb, TransformComponent& transform) {
			if (rb.dynamicActor) {
				PxTransform pxTransform = rb.dynamicActor->getGlobalPose();
				transform.SetLocalPosition(Physics::PxVec3toGlmVec3(pxTransform.p));
			}
		});

		//auto view = m_scene->Reg().view<TransformComponent, RigidBodyComponent>();
		//for (auto e : view)
		//{
		//	auto [transform, rigidBodyComponent] = view.get<TransformComponent, RigidBodyComponent>(e);
		//	Entity ent = Entity({ e, m_scene });
		//
		//	bool isTrigger = false;
		//	if (!rigidBodyComponent.GetRigidBody())
		//	{
		//		continue;
		//	}
		//
		//	if (rigidBodyComponent.Rigidbody->IsTrigger())
		//	{
		//		auto globaltransform = transform.GetGlobalMatrix();
		//		Vec3 translation;
		//		glm::quat rotation;
		//		Vec3 scale;
		//		Vec3 skew;
		//		Vec4 perspective;
		//		glm::decompose(globaltransform, scale, rotation, translation, skew, perspective);
		//
		//		PhysicsManager::Get().SetBodyTransform(ent, globaltransform[3], glm::normalize(transform.GetGlobalRotation()));
		//	}
		//}

	}

	void PhysicsSystem::Exit()
	{
		//PhysicsManager::Get().Reset();
	}

	/*void PhysicsSystem::InitializeNewBodies()
	{
		InitializeShapes();
		InitializeRigidbodies();
		InitializeCharacterControllers();
	}

	void PhysicsSystem::InitializeQuakeMap()
	{
		auto quakeBrushesView = m_scene->Reg().view<TransformComponent, BSPBrushComponent>();
		for (auto e : quakeBrushesView)
		{
			const auto [transformComponent, brushComponent] = quakeBrushesView.get<TransformComponent, BSPBrushComponent>(e);

			// Doesn't apply to non-solid brushes
			if ((!brushComponent.IsSolid && !brushComponent.IsTrigger))
			{
				continue;
			}

			for (const auto& hull : brushComponent.Hulls)
			{
				const auto entity = CreateRef<Entity>( e, m_scene );
				const Vec3& startPosition = transformComponent.GetGlobalPosition();
				const Matrix4& startTransform = transformComponent.GetGlobalMatrix();
				const Quat& startRotation = transformComponent.GetGlobalRotation();
				const auto collisionShape = CreateRef<Physics::ConvexHullShape>(hull);

				auto rigidBody = CreateRef<Physics::RigidBody>(0.0f, startPosition, startRotation, startTransform, collisionShape, entity, Vec3{ 0, 0, 0 }, brushComponent.IsFunc);
				brushComponent.Rigidbody.push_back(rigidBody);
				rigidBody->SetIsTrigger(brushComponent.IsTrigger);
				PhysicsManager::Get().RegisterBody(rigidBody);
			}
		}
	}

	void PhysicsSystem::InitializeShapes()
	{
		const auto boxView = m_scene->Reg().view<BoxColliderComponent>();
		for (auto entity : boxView)
		{
			auto& boxComponent = boxView.get<BoxColliderComponent>(entity);
			if (!boxComponent.Box) {
				boxComponent.Box = CreateRef<Physics::Box>(boxComponent.Size);
			}
		}

		//const auto capsuleView = m_scene->m_Registry.view<CapsuleColliderComponent>();
		//for (auto entity : capsuleView)
		//{
		//	auto& capsuleComponent = capsuleView.get<CapsuleColliderComponent>(entity);
		//	if (!capsuleComponent.Capsule)
		//	{
		//		float radius = capsuleComponent.Radius;
		//		float height = capsuleComponent.Height;
		//		capsuleComponent.Capsule = CreateRef<Physics::Capsule>(radius, height);
		//	}
		//}

		//const auto cylinderView = m_Scene->m_Registry.view<CylinderColliderComponent>();
		//for (auto entity : cylinderView)
		//{
		//	auto& cylinderComponent = cylinderView.get<CylinderColliderComponent>(entity);
		//	if (!cylinderComponent.Cylinder)
		//	{
		//		float radius = cylinderComponent.Radius;
		//		float height = cylinderComponent.Height;
		//		cylinderComponent.Cylinder = CreateRef<Physics::Cylinder>(radius, height);
		//	}
		//}

		const auto sphereView = m_scene->Reg().view<SphereColliderComponent>();
		for (auto entity : sphereView)
		{
			auto& sphereComponent = sphereView.get<SphereColliderComponent>(entity);
			if (!sphereComponent.Sphere) {
				sphereComponent.Sphere = CreateRef<Physics::Sphere>(sphereComponent.Radius);
			}
		}

		//const auto meshColliderView = m_Scene->m_Registry.view<MeshColliderComponent>();
		//for (auto e : meshColliderView)
		//{
		//	Entity entity{ e, m_Scene };
		//	if (!entity.HasComponent<ModelComponent>())
		//	{
		//		Logger::Log("Cannot use mesh collider without model component", "physics", WARNING);
		//	}
		//
		//	auto& meshColliderComponent = meshColliderView.get<MeshColliderComponent>(e);
		//	if (!meshColliderComponent.Shape)
		//	{
		//		const auto& modelComponent = entity.GetComponent<ModelComponent>();
		//
		//		if (modelComponent.ModelResource)
		//		{
		//			uint32_t subMeshId = meshColliderComponent.SubMesh;
		//			const std::vector<Ref<Mesh>>& submeshes = modelComponent.ModelResource->GetMeshes();
		//			if (subMeshId >= submeshes.size())
		//			{
		//				Logger::Log("Cannot create mesh collider, invalid submesh ID", "physics", WARNING);
		//			}
		//
		//			Ref<Mesh> mesh = submeshes[subMeshId];
		//			meshColliderComponent.Shape = CreateRef<Physics::MeshShape>(mesh);
		//		}
		//	}
		//}
	}

	void PhysicsSystem::InitializeRigidbodies()
	{
		auto view = m_scene->Reg().view<TransformComponent, RigidBodyComponent>();
		for (auto e : view)
		{
			auto [transform, rigidBodyComponent] = view.get<TransformComponent, RigidBodyComponent>(e);
			Ref<Entity> ent = CreateRef<Entity>( e, m_scene );
			Ref<Physics::RigidBody> rigidBody;
			Ref<Physics::PhysicShape> shape;

			bool isTrigger = false;
			if (rigidBodyComponent.GetRigidBody())
			{
				continue;
			}

			if (ent->HasComponent<BoxColliderComponent>())
			{
				BoxColliderComponent& boxComponent = ent->GetComponent<BoxColliderComponent>();
				isTrigger = boxComponent.IsTrigger;
				shape = CreateRef<Physics::Box>(boxComponent.Size);
			}

			//if (ent.HasComponent<CapsuleColliderComponent>())
			//{
			//	auto& capsuleComponent = ent.GetComponent<CapsuleColliderComponent>();
			//	float radius = capsuleComponent.Radius;
			//	float height = capsuleComponent.Height;
			//	isTrigger = capsuleComponent.IsTrigger;
			//	shape = CreateRef<Physics::Capsule>(radius, height);
			//}
			//
			//if (ent.HasComponent<CylinderColliderComponent>())
			//{
			//	auto& cylinderComponent = ent.GetComponent<CylinderColliderComponent>();
			//	float radius = cylinderComponent.Radius;
			//	float height = cylinderComponent.Height;
			//	isTrigger = cylinderComponent.IsTrigger;
			//	shape = CreateRef<Physics::Cylinder>(radius, height);
			//}

			if (ent->HasComponent<SphereColliderComponent>())
			{
				const auto& component = ent->GetComponent<SphereColliderComponent>();
				isTrigger = component.IsTrigger;
				shape = CreateRef<Physics::Sphere>(component.Radius);
			}

			//if (ent.HasComponent<MeshColliderComponent>())
			//{
			//	if (!ent.HasComponent<ModelComponent>())
			//	{
			//		Logger::Log("Cannot use mesh collider without model component", "physics", WARNING);
			//	}
			//
			//	const auto& modelComponent = ent.GetComponent<ModelComponent>();
			//	const auto& component = ent.GetComponent<MeshColliderComponent>();
			//
			//	isTrigger = component.IsTrigger;
			//
			//	if (modelComponent.ModelResource)
			//	{
			//		uint32_t subMeshId = component.SubMesh;
			//		const std::vector<Ref<Mesh>>& submeshes = modelComponent.ModelResource->GetMeshes();
			//		if (subMeshId >= submeshes.size())
			//		{
			//			Logger::Log("Cannot create mesh collider, invalid submesh ID", "physics", WARNING);
			//		}
			//
			//		Ref<Mesh> mesh = submeshes[subMeshId];
			//		shape = CreateRef<Physics::MeshShape>(mesh);
			//	}
			//}

			if (!shape)
			{
				continue;
			}

			rigidBody = CreateRef<Physics::RigidBody>(rigidBodyComponent.Mass, transform.GetGlobalPosition(), transform.GetGlobalRotation(), transform.GetGlobalMatrix(), shape, ent);
			rigidBody->SetLockXAxis(rigidBodyComponent.LockX);
			rigidBody->SetLockYAxis(rigidBodyComponent.LockY);
			rigidBody->SetLockZAxis(rigidBodyComponent.LockZ);

			rigidBody->SetIsTrigger(isTrigger);

			PhysicsManager::Get().RegisterBody(rigidBody);

			rigidBodyComponent.Rigidbody = rigidBody;
		}
	}

	void PhysicsSystem::InitializeCharacterControllers()
	{
		auto characterControllerView = m_scene->Reg().view<TransformComponent, CharacterControllerComponent>();
		for (const auto& e : characterControllerView)
		{
			Ref<Entity> entity = CreateRef<Entity>( e, m_scene );
			auto [transformComponent, characterControllerComponent] = characterControllerView.get<TransformComponent, CharacterControllerComponent>(e);

			if (characterControllerComponent.GetCharacterController()) {
				continue;
			}

			Ref<Physics::PhysicShape> shape;
			bool hasValidShape = true;
			//if (entity.HasComponent<CapsuleColliderComponent>())
			//{
			//	const auto& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();
			//	shape = capsuleColliderComponent.Capsule;
			//}
			if (entity->HasComponent<BoxColliderComponent>())
			{
				const auto& capsuleColliderComponent = entity->GetComponent<BoxColliderComponent>();
				shape = capsuleColliderComponent.Box;
			}
			else if (entity->HasComponent<SphereColliderComponent>())
			{
				const auto& capsuleColliderComponent = entity->GetComponent<SphereColliderComponent>();
				shape = capsuleColliderComponent.Sphere;
			}
			//else if (entity.HasComponent<CylinderColliderComponent>())
			//{
			//	const auto& cylinderColliderComponent = entity.GetComponent<CylinderColliderComponent>();
			//	shape = cylinderColliderComponent.Cylinder;
			//}
			else
			{
				hasValidShape = false;
				HZ_CORE_WARN("No shape was provided for character controller. Default one was provided.");
				shape = CreateRef<Physics::Capsule>(1.0f, 0.5f);
			}

			Physics::CharacterControllerSettings settings
			{
				shape,
				characterControllerComponent.Friction,
				characterControllerComponent.MaxSlopeAngle,
				characterControllerComponent.AutoStepping,
				characterControllerComponent.StickToFloorStepDown,
				characterControllerComponent.StepDownExtra,
				characterControllerComponent.SteppingStepUp,
				characterControllerComponent.SteppingMinDistance,
				characterControllerComponent.SteppingForwardDistance
			};

			auto characterController = CreateRef<Physics::CharacterController>(std::move(settings));
			characterController->SetEntity(*entity.get()); // Used to link back to the entity
			characterController->Position = transformComponent.GetGlobalPosition();
			characterController->Rotation = transformComponent.GetGlobalRotation();
			characterControllerComponent.SetCharacterController(characterController);

			PhysicsManager::Get().RegisterCharacterController(characterController);
		}
	}

	void PhysicsSystem::ApplyForces()
	{
		auto view = m_scene->Reg().view<TransformComponent, RigidBodyComponent>();
		for (auto e : view)
		{
			auto [transform, rigidBodyComponent] = view.get<TransformComponent, RigidBodyComponent>(e);
			Ref<Entity> ent = CreateRef<Entity>( e, m_scene );
			Ref<Physics::RigidBody> rigidBody;

			// Not initialized yet.
			if (!rigidBodyComponent.GetRigidBody() || rigidBodyComponent.QueuedForce == Vec3() || rigidBodyComponent.Mass == 0.0)
			{
				continue;
			}

			PhysicsManager::Get().GetWorld()->AddForceToRigidBody(*ent.get(), rigidBodyComponent.QueuedForce);

			rigidBodyComponent.QueuedForce = Vec3();
		}
	}*/
}