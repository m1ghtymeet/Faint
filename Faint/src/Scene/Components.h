#pragma once

#include "Components/NameComponent.h"
#include "Components/VisibilityComponent.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CapsuleColliderComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/NetScriptComponent.h"
#include "Components/WrenScriptComponent.h"
#include "Components/LuaScriptComponent.h"
#include "Components/LightComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/BSPBrushComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SkyComponent.h"
#include "Components/TextComponent.h"
#include "Components/TextBlitterComponent.h"

namespace Faint {

	template<typename... Component>
	struct ComponentGroup { };

	class ParentComponent;
	using AllComponents =
		ComponentGroup<VisibilityComponent, TransformComponent, ParentComponent,
		CameraComponent, NetScriptComponent, WrenScriptComponent, LuaScriptComponent,
		BoxColliderComponent, SphereColliderComponent, CharacterControllerComponent, RigidBodyComponent,
		AudioComponent, MeshRendererComponent, LightComponent, TextComponent, TextBlitterComponent>;
}