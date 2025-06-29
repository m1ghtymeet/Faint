#pragma once
#include <cstdint>
#include <string>

enum class RendererAPI { OPENGL = 0, DIRECTX = 1, VULKAN = 2, UNDEFINED = 3 };

enum class ImageDataType {
	UNCOMPRESSED,
	COMPRESSED,
	UNDEFINED
};

enum class TextureWrapMode {
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER
};

enum class TextureFilter {
	LINEAR,
	NEAREST,
	LINEAR_MIPMAP
};

enum class ShadingMode {
	SHADED,
	WIREFRAME,
	WIREFRAME_OVERLAY
};

enum DebugRenderMode {
	NONE = 0,
	PATHFINDING_RECAST,
	PHYSX_ALL,
	PHYSX_RAYCAST,
	PHYSX_COLLISION,
};

enum class ShaderDataType : uint8_t {
	NONE = 0,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	MAT3,
	MAT4,
	INT,
	INT2,
	INT3,
	INT4,
	BOOL
};

enum struct PhysicsType {
	NONE = 0,
	RIGID_DYNAMIC,
	RIGID_STATIC,
	UNDEFINED
};

struct PhysicsUserData {
	PhysicsType physicsType = PhysicsType::NONE;
};

struct PhysXRayResult {
	PhysicsUserData userData;
	std::string hitObjectName;
};

enum CollisionGroup {
	NO_COLLISION = 0,
	CHARACTER_CONTROLLER,
	DYNAMIC_OBJECT,
	STATIC_OBJECT
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

enum class EditorMode {
	IDLE = 0,
	HOUSE_EDITOR,
	MAP_EDITOR,
	SECTOR_EDITOR
};

enum class RendererOverrideState {
	NONE = 0,
	BASE_COLOR,
	NORMALS,
	RMA
};