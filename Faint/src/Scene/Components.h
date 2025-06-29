#pragma once

#include "Components/CTransform.h"
#include "Components/CCamera.h"
#include "Components/CModelRenderer.h"
#include "Components/SkinnedMeshRendererComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CBoxCollider.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CCapsuleCollider.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/NetScriptComponent.h"
#include "Components/WrenScriptComponent.h"
#include "Components/LuaScriptComponent.h"
#include "Components/CLight.h"
#include "Components/CAudio.h"
#include "Components/SkyComponent.h"

namespace Faint {

	template<typename... Component>
	struct ComponentGroup { };

	using AllComponents =
		ComponentGroup<TransformComponent,
		CameraComponent, NetScriptComponent, WrenScriptComponent, LuaScriptComponent,
		BoxColliderComponent, SphereColliderComponent, CharacterControllerComponent, RigidBodyComponent,
		AudioComponent, SkinnedMeshRendererComponent, MeshRendererComponent, LightComponent,
		SkyComponent>;
}