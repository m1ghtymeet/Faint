#pragma once

#include "Component.h"

#include "TransformComponent.h"
#include "Physics/Physics.h"
#include "Core/Base.h"

namespace Faint {
	struct RigidBodyComponent : public Component {
		//FAINTCOMPONENT(RigidBodyComponent, "Rigid Body")
		
		//static void InitializeComponentClass()
		//{
		//	BindComponentField<&RigidBodyComponent::Mass>("Mass", "Mass");
		//	BindComponentField<&RigidBodyComponent::LockX>("LockX", "Lock X");
		//	BindComponentField<&RigidBodyComponent::LockY>("LockY", "Lock Y");
		//	BindComponentField<&RigidBodyComponent::LockZ>("LockZ", "Lock Z");
		//}
	public:
		PxRigidDynamic* dynamicActor = nullptr;
		bool isKinematic = false;
		float mass;

		RigidBodyComponent();
		PxRigidDynamic* GetRigidBody() const;

		json Serialize()
		{
			BEGIN_SERIALIZE();

			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{

			return true;
		}
	};
}