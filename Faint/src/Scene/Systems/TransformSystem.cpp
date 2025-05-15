#include "TransformSystem.h"

#include "Math/Math.h"
#include "Scene/Scene.h"
#include <Scene/Components/TransformComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/ParentComponent.h>

namespace Faint
{
	TransformSystem::TransformSystem(Scene* scene)
	{
		m_scene = scene;
	}

	bool TransformSystem::Init()
	{
		UpdateTransform();
		return true;
	}

	void TransformSystem::Update(Time ts)
	{
		UpdateTransform();
	}

	void TransformSystem::FixedUpdate(Time ts)
	{

	}

	void TransformSystem::Exit()
	{

	}

	void TransformSystem::UpdateTransform()
	{
		// Calculate all local transforms
		auto localTransformView = m_scene->Reg().view<TransformComponent>();
		for (auto tv : localTransformView)
		{
			TransformComponent& transform = localTransformView.get<TransformComponent>(tv);
			Entity currentEntity = { tv, m_scene };
			ParentComponent& parentComponent = currentEntity.GetComponent<ParentComponent>();
			if (transform.Dirty)
			{
				const Vec3& localTranslate = transform.GetLocalPosition();
				const Quat& localRot = glm::normalize(transform.GetLocalRotation());
				const Vec3& localScale = transform.GetLocalScale();
				const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localTranslate);
				const Matrix4& rotationMatrix = glm::mat4_cast(localRot);
				const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
				const Matrix4& newLocalTransform = translationMatrix * rotationMatrix * scaleMatrix;

				transform.GlobalDirty = true;
				UpdateDirtyFlagRecursive(currentEntity);

				transform.SetLocalMatrix(newLocalTransform);
				transform.Dirty = false;
			}
		}

		// Calculate all global transforms
		auto transformView = m_scene->Reg().view<TransformComponent, ParentComponent>();
		for (auto e : transformView)
		{
			auto [transform, parent] = transformView.get<TransformComponent, ParentComponent>(e);

			Entity currentParent = Entity((entt::entity)e, m_scene);

			Matrix4 globalTransform = transform.GetLocalMatrix();
			Vec3 globalPosition = transform.GetLocalPosition();
			Quat globalOrientation = transform.GetLocalRotation();
			Vec3 globalScale = transform.GetLocalScale();
			ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();

			globalTransform = parentComponent.HasParent ? parentComponent.Parent.GetComponent<TransformComponent>().GetGlobalMatrix() * globalTransform : globalTransform;

			Math::DecomposeTransform(globalTransform, globalPosition, globalOrientation, globalScale);

			transform.SetGlobalPosition(globalPosition);
			transform.SetGlobalRotation(globalOrientation);
			transform.SetGlobalScale(globalScale);
			transform.SetGlobalMatrix(globalTransform);

			if (!parent.HasParent)
			{
				// If no parents, then globalTransform is local transform.
				transform.SetGlobalMatrix(transform.GetLocalMatrix());
				transform.SetGlobalPosition(transform.GetLocalPosition());
				transform.SetGlobalRotation(transform.GetLocalRotation());
				transform.SetGlobalScale(transform.GetLocalScale());
				continue;
			}
			
			//			ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();
			//#define FRAME_PERFECT_TRANSFORM
			//#ifndef FRAME_PERFECT_TRANSFORM
			//			if (parentComponent.HasParent)
			//			{
			//				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
			//
			//				globalPosition = transformComponent.GetGlobalPosition() + (globalPosition);
			//
			//				globalScale *= transformComponent.GetGlobalScale();
			//				globalOrientation = transformComponent.GetGlobalRotation() * globalOrientation;
			//				globalTransform = transformComponent.GetGlobalTransform() * globalTransform;
			//			}
			//#else
			//			bool exitEarly = false;
			//			while (parentComponent.HasParent)
			//			{
			//				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
			//				
			//				globalPosition = transformComponent.GetLocalPosition() + (globalPosition);
			//				globalScale *= transformComponent.GetLocalScale();
			//				globalOrientation = transformComponent.GetLocalRotation() * globalOrientation;
			//				globalTransform = transformComponent.GetLocalMatrix() * globalTransform;
			//				transformComponent.GlobalDirty = false;
			//			
			//				//NameComponent& nameComponent = parentComponent.Parent.GetComponent<NameComponent>();
			//				
			//				parentComponent = parentComponent.Parent.GetComponent<ParentComponent>();
			//			}
			//#endif // FRAME_PERFECT_TRANSFORM
			//
			transform.SetGlobalPosition(globalPosition);
			transform.SetGlobalRotation(globalOrientation);
			transform.SetGlobalScale(globalScale);
			transform.SetGlobalMatrix(globalTransform);
		}

		auto camView = m_scene->Reg().view<TransformComponent, CameraComponent, ParentComponent>();
		for (auto& e : camView)
		{
			auto [transform, camera, parent] = camView.get<TransformComponent, CameraComponent, ParentComponent>(e);
			
			//const Matrix4& cameraTransform = camera.camera->GetViewMatrix();

			Matrix4 globalTransform = transform.GetLocalMatrix();
			Vec3 globalPosition = transform.GetLocalPosition();
			Quat globalRotation = transform.GetLocalRotation();
			Vec3 globalScale = transform.GetLocalScale();

			glm::mat4 globalMatrix = parent.HasParent ? parent.Parent.GetComponent<TransformComponent>().GetLocalMatrix() * globalTransform : globalTransform;
			// Don't use this now
			//Math::DecomposeTransform(globalTransform, globalPosition, globalRotation, globalScale);

			if (parent.HasParent) {
				globalPosition = parent.Parent.GetComponent<TransformComponent>().GetLocalPosition() + globalPosition;
				globalRotation = parent.Parent.GetComponent<TransformComponent>().GetLocalRotation() * globalRotation;
				parent.Parent.GetComponent<TransformComponent>().GlobalDirty = false;
				globalMatrix = parent.Parent.GetComponent<TransformComponent>().GetLocalMatrix() * globalTransform;
				//std::cout << parent.Parent.GetComponent<TransformComponent>().GetLocalPosition().y << "\n";
			}

			//while (parent.HasParent) {
			//	TransformComponent& transformComponent = parent.Parent.GetComponent<TransformComponent>();
			//	
			//	globalPosition = transformComponent.GetLocalPosition() + (globalPosition);
			//	globalScale *= transformComponent.GetLocalScale();
			//	globalRotation = transformComponent.GetLocalRotation() * globalRotation;
			//	globalTransform = transformComponent.GetLocalMatrix() * globalTransform;
			//	transformComponent.GlobalDirty = false;
			//	
			//	parent = parent.Parent.GetComponent<ParentComponent>();
			//}

			//transform.SetGlobalPosition(globalPosition);
			//transform.SetGlobalRotation(globalRotation);
			//transform.SetGlobalScale(globalScale);
			//transform.SetGlobalMatrix(globalTransform);

			camera.camera->SetPosition(globalPosition);
			camera.camera->SetRotationQ(globalRotation);
			camera.camera->Update();
			
			const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), globalPosition);
			const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
			const Vec4& forward = Vec4(0, 0, -1, 1);
			const Vec4& globalForward = rotationMatrix * forward;

			//const Vec4& right = Vec4(1, 0, 0, 1);
			//const Vec4& globalRight = rotationMatrix * right;
			//camera.camera->m_forward = globalForward;
			//camera.camera->m_right = globalRight;
			//camera.camera->SetViewMatrix(glm::inverse(translationMatrix * rotationMatrix));
		}
	}

	void TransformSystem::UpdateDirtyFlagRecursive(Entity& entity)
	{
		auto& parentComponent = entity.GetComponent<ParentComponent>();

		for (auto& c : parentComponent.Children)
		{
			auto& childParentComponent = c.GetComponent<TransformComponent>();
			childParentComponent.GlobalDirty = true;

			UpdateDirtyFlagRecursive(c);
		}
	}

	void TransformSystem::CalculateGlobalTransform(Entity& entity)
	{
		auto& parentComponent = entity.GetComponent<ParentComponent>();
		auto& transformComponent = entity.GetComponent<TransformComponent>();
		auto& parentTransformComponent = parentComponent.Parent.GetComponent<TransformComponent>();

		Vec3 globalPosition = parentTransformComponent.GetGlobalPosition() + Vec3(transformComponent.GetLocalPosition());
		Quat globalRotation = parentTransformComponent.GetGlobalRotation() * transformComponent.GetLocalRotation();
		Vec3 globalScale = parentTransformComponent.GetGlobalScale() * transformComponent.GetGlobalScale();
		auto globalTransform = transformComponent.GetGlobalMatrix() * parentTransformComponent.GetGlobalMatrix();

		transformComponent.SetGlobalPosition(Vec3(globalPosition));
		transformComponent.SetGlobalRotation(globalRotation);
		transformComponent.SetGlobalScale(globalScale);
		transformComponent.SetGlobalMatrix(globalTransform);
		transformComponent.GlobalDirty = false;

		for (auto& c : parentComponent.Children)
		{
			CalculateGlobalTransform(c);
		}
	}
}