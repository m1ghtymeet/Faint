#include "TransformSystem.h"

#include <Engine.h>
#include "Math/Math.h"
#include "Scene/Scene.h"
#include <Scene/Components/CTransform.h>
#include <Scene/Components/CCamera.h>

namespace Faint {

	std::unordered_map<int, Transform> g_initialTransform;

	TransformSystem::TransformSystem(Scene* scene) {
		m_scene = scene;
	}

	bool TransformSystem::Init() {
		g_initialTransform.clear();
		for (auto e : m_scene->GetAllEntities()) {
			//auto& transform = e->transform;
			//g_initialTransform[e->GetID()] = Transform{
			//	transform.GetLocalPosition(),
			//	transform.GetLocalRotation(),
			//	transform.GetLocalScale()
			//};
		}
		UpdateTransform();
		return true;
	}

	void TransformSystem::Update(Time ts) {
		UpdateTransform();
	}

	void TransformSystem::FixedUpdate(Time ts) {
	}

	void TransformSystem::Exit() {
		//for (auto& [entity, state] : g_initialTransform) {
		//	if (!m_scene->Reg().valid(entity)) continue;
		//	auto& transform = m_scene->Reg().get<TransformComponent>(entity);
		//	transform.SetLocalPosition(state.m_localPosition);
		//	transform.SetLocalRotation(state.m_localRotation);
		//	transform.SetLocalScale(state.m_localScale);
		//	transform.Dirty = true;
		//}
	}

	void TransformSystem::UpdateTransform() {
		// Calculate all local transforms
		//for (auto* tv : Engine::GetCurrentScene()->GetAllEntities()) {
		//	TransformComponent* transform = tv->GetComponent<TransformComponent>();
		//	const Vec3& localTranslate = transform->GetLocalPosition();
		//	const Quat& localRot = glm::normalize(transform->GetLocalRotation());
		//	const Vec3& localScale = transform->GetLocalScale();
		//	const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localTranslate);
		//	const Matrix4& rotationMatrix = glm::mat4_cast(localRot);
		//	const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
		//	const Matrix4& newLocalTransform = translationMatrix * rotationMatrix * scaleMatrix;
		//
		//	//UpdateDirtyFlagRecursive(*currentEntity);
		//
		//	transform->SetLocalMatrix(newLocalTransform);
		//}

		// Calculate all global transforms
		//auto transformView = m_scene->Reg().view<TransformComponent>();
		//for (auto e : transformView)
		//{
		//	auto [transform, parent] = transformView.get<TransformComponent, ParentComponent>(e);
		//
		//	Entity currentParent = Entity((entt::entity)e, m_scene);
		//
		//	Matrix4 globalTransform = transform.GetLocalMatrix();
		//	Vec3 globalPosition = transform.GetLocalPosition();
		//	Quat globalOrientation = transform.GetLocalRotation();
		//	Vec3 globalScale = transform.GetLocalScale();
		//	ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();
		//
		//	globalTransform = parentComponent.HasParent ? parentComponent.Parent.GetComponent<TransformComponent>().GetGlobalMatrix() * globalTransform : globalTransform;
		//
		//	Math::DecomposeTransform(globalTransform, globalPosition, globalOrientation, globalScale);
		//
		//	transform.SetGlobalPosition(globalPosition);
		//	transform.SetGlobalRotation(globalOrientation);
		//	transform.SetGlobalScale(globalScale);
		//	transform.SetGlobalMatrix(globalTransform);
		//
		//	if (!parent.HasParent)
		//	{
		//		// If no parents, then globalTransform is local transform.
		//		transform.SetGlobalMatrix(transform.GetLocalMatrix());
		//		transform.SetGlobalPosition(transform.GetLocalPosition());
		//		transform.SetGlobalRotation(transform.GetLocalRotation());
		//		transform.SetGlobalScale(transform.GetLocalScale());
		//		continue;
		//	}
		//	
		//	//			ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();
		//	//#define FRAME_PERFECT_TRANSFORM
		//	//#ifndef FRAME_PERFECT_TRANSFORM
		//	//			if (parentComponent.HasParent)
		//	//			{
		//	//				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
		//	//
		//	//				globalPosition = transformComponent.GetGlobalPosition() + (globalPosition);
		//	//
		//	//				globalScale *= transformComponent.GetGlobalScale();
		//	//				globalOrientation = transformComponent.GetGlobalRotation() * globalOrientation;
		//	//				globalTransform = transformComponent.GetGlobalTransform() * globalTransform;
		//	//			}
		//	//#else
		//	//			bool exitEarly = false;
		//	//			while (parentComponent.HasParent)
		//	//			{
		//	//				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
		//	//				
		//	//				globalPosition = transformComponent.GetLocalPosition() + (globalPosition);
		//	//				globalScale *= transformComponent.GetLocalScale();
		//	//				globalOrientation = transformComponent.GetLocalRotation() * globalOrientation;
		//	//				globalTransform = transformComponent.GetLocalMatrix() * globalTransform;
		//	//				transformComponent.GlobalDirty = false;
		//	//			
		//	//				//NameComponent& nameComponent = parentComponent.Parent.GetComponent<NameComponent>();
		//	//				
		//	//				parentComponent = parentComponent.Parent.GetComponent<ParentComponent>();
		//	//			}
		//	//#endif // FRAME_PERFECT_TRANSFORM
		//	//
		//	transform.SetGlobalPosition(globalPosition);
		//	transform.SetGlobalRotation(globalOrientation);
		//	transform.SetGlobalScale(globalScale);
		//	transform.SetGlobalMatrix(globalTransform);
		//}

		//auto camView = m_scene->Reg().view<TransformComponent, CameraComponent>();
		//for (auto& e : camView)
		//{
		//	auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(e);
		//	
		//	//const Matrix4& cameraTransform = camera.camera->GetViewMatrix();
		//
		//	Matrix4 globalTransform = transform.GetLocalMatrix();
		//	Vec3 globalPosition = transform.GetLocalPosition();
		//	Quat globalRotation = transform.GetLocalRotation();
		//	Vec3 globalScale = transform.GetLocalScale();
		//
		//	glm::mat4 globalMatrix = parent.HasParent ? parent.Parent.GetComponent<TransformComponent>().GetLocalMatrix() * globalTransform : globalTransform;
		//	// Don't use this now
		//	//Math::DecomposeTransform(globalTransform, globalPosition, globalRotation, globalScale);
		//
		//	if (parent.HasParent) {
		//		globalPosition = parent.Parent.GetComponent<TransformComponent>().GetLocalPosition() + globalPosition;
		//		globalRotation = parent.Parent.GetComponent<TransformComponent>().GetLocalRotation() * globalRotation;
		//		parent.Parent.GetComponent<TransformComponent>().GlobalDirty = false;
		//		globalMatrix = parent.Parent.GetComponent<TransformComponent>().GetLocalMatrix() * globalTransform;
		//		//std::cout << parent.Parent.GetComponent<TransformComponent>().GetLocalPosition().y << "\n";
		//	}
		//
		//	//while (parent.HasParent) {
		//	//	TransformComponent& transformComponent = parent.Parent.GetComponent<TransformComponent>();
		//	//	
		//	//	globalPosition = transformComponent.GetLocalPosition() + (globalPosition);
		//	//	globalScale *= transformComponent.GetLocalScale();
		//	//	globalRotation = transformComponent.GetLocalRotation() * globalRotation;
		//	//	globalTransform = transformComponent.GetLocalMatrix() * globalTransform;
		//	//	transformComponent.GlobalDirty = false;
		//	//	
		//	//	parent = parent.Parent.GetComponent<ParentComponent>();
		//	//}
		//
		//	//transform.SetGlobalPosition(globalPosition);
		//	//transform.SetGlobalRotation(globalRotation);
		//	//transform.SetGlobalScale(globalScale);
		//	//transform.SetGlobalMatrix(globalTransform);
		//
		//	camera.camera->SetPosition(globalPosition);
		//	camera.camera->SetRotationQ(globalRotation);
		//	camera.camera->Update();
		//	
		//	const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), globalPosition);
		//	const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
		//	const Vec4& forward = Vec4(0, 0, -1, 1);
		//	const Vec4& globalForward = rotationMatrix * forward;
		//}
	}

	void TransformSystem::UpdateDirtyFlagRecursive(Entity& entity)
	{
	}

	void TransformSystem::CalculateGlobalTransform(Entity& entity)
	{
		auto* transformComponent = entity.GetComponent<TransformComponent>();


	}
}