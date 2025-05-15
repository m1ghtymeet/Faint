#pragma once
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <iostream>

#define MAX_LIGHTS 64

#define ORANGE              glm::vec4(1.00f, 0.65f, 0.00f, 1.0f)
#define BLACK               glm::vec4(0.00f, 0.00f, 0.00f, 1.0f)
#define WHITE               glm::vec4(1.00f, 1.00f, 1.00f, 1.0f)
#define RED                 glm::vec4(1.00f, 0.00f, 0.00f, 1.0f)
#define GREEN               glm::vec4(0.00f, 1.00f, 0.00f, 1.0f)
#define BLUE                glm::vec4(0.00f, 0.00f, 1.00f, 1.0f)
#define YELLOW              glm::vec4(1.00f, 1.00f, 0.00f, 1.0f)
#define PURPLE              glm::vec4(1.00f, 0.00f, 1.00f, 1.0f)
#define GREY                glm::vec4(0.25f, 0.25f, 0.25f, 1.0f)
#define LIGHT_BLUE          glm::vec4(0.00f, 1.00f, 1.00f, 1.0f)
#define LIGHT_GREEN         glm::vec4(0.16f, 0.78f, 0.23f, 1.0f)
#define LIGHT_RED           glm::vec4(0.80f, 0.05f, 0.05f, 1.0f)
#define TRANSPARENT         glm::vec4(0.00f, 0.00f, 0.00f, 0.0f)
#define GRID_COLOR          glm::vec4(0.50f, 0.50f, 0.60f, 1.0f)
#define OUTLINE_COLOR       glm::vec4(1.00f, 0.50f, 0.00f, 0.0f)
#define DEFAULT_LIGHT_COLOR glm::vec4(1.00f, 0.7799999713897705f, 0.5289999842643738f, 1.0f)

enum class RendererAPI { UNDEFINED = 0, OPENGL = 1, VULKAN = 2 };

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

enum DebugRenderMode {
	NONE = 0,
	PATHFINDING_RECAST,
	PHYSX_ALL,
	PHYSX_RAYCAST,
	PHYSX_COLLISION,
};

#define PI 3.141592653589793f

#define SMALL_NUMBER		(float)9.99999993922529e-9

struct Transform2 {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);

	Transform2() = default;

	explicit Transform2(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}

	glm::mat4 to_mat4() {
		glm::mat4 m = glm::translate(glm::mat4(1), position);
		m *= glm::mat4_cast(glm::quat(rotation));
		m = glm::scale(m, scale);
		return m;
	}
	glm::vec3 to_forward_vector() {
		glm::quat q = glm::quat(rotation);
		return glm::normalize(q * glm::vec3(0.0f, 0.0f, 1.0f));
	}
	glm::vec3 to_right_vector() {
		glm::quat q = glm::quat(rotation);
		return glm::normalize(q * glm::vec3(1.0f, 0.0f, 0.0f));
	}
};

struct Vertex {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 normal = glm::vec3(0);
	glm::vec2 uv = glm::vec3(0);
	glm::vec3 tangent = glm::vec3(0);
	bool operator==(const Vertex& other) const {
		return position == other.position && normal == other.normal && uv == other.uv;
	}
};

struct WeightedVertex {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 normal = glm::vec3(0);
	glm::vec2 uv = glm::vec3(0);
	glm::vec3 tangent = glm::vec3(0);
	glm::ivec4 boneID = glm::ivec4(0);
	glm::vec4 weight = glm::vec4(0);
	bool operator==(const Vertex& other) const {
		return position == other.position && normal == other.normal && uv == other.uv;
	}
	bool operator==(const WeightedVertex& other) const {
		return position == other.position && normal == other.normal && uv == other.uv;
	}
};

#pragma pack(push, 1)
struct DebugVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::ivec2 pixelOffset;
};
#pragma pack(pop)

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t h1 = hash<glm::vec3>()(vertex.position);
			size_t h2 = hash<glm::vec3>()(vertex.normal);
			size_t h3 = hash<glm::vec2>()(vertex.uv);
			return h1 ^ (h2 << 1) ^ (h3 << 2);  // Combining the hashes
		}
	};
}

struct TextureData {
	int m_width = 0;
	int m_height = 0;
	int m_numChannels = 0;
	void* m_data = nullptr;
};

struct Node {
	std::string name;
	int parentIndex;
	glm::mat4 inverseBindTransform;
};

namespace Faint {
	using MouseCode = uint16_t;
	namespace Mouse {
		enum : MouseCode {
			// From glfw3.h
			Button0    = 0,
			Button1    = 1,
			Button2    = 2,
			Button3    = 3,
			Button4    = 4,
			Button5    = 5,
			Button6    = 6,
			Button7    = 7,

			ButtonLast = Button7,
			ButtonLeft = Button0,
			ButtonRight = Button1,
			ButtonMiddle = Button2
		};
	}
}

/* ^^^ ENUM ^^^ */
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
	ENVIROMENT_OBSTACLE,
	GENERIC_BOUNCEABLE,
	PLAYER
};

enum RaycastGroup {
	RAYCAST_DISABLED = 0,
	RAYCAST_ENABLED
};

struct PhysicsFilterData {
	RaycastGroup raycastGroup = RaycastGroup::RAYCAST_DISABLED;
	CollisionGroup collisionGroup = CollisionGroup::NO_COLLISION;
	CollisionGroup collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;
};

#pragma pack(push, 1)
struct BvhNode {
	glm::vec3 boundsMin;
	uint32_t firstChildOrPrimitive;
	glm::vec3 boundsMax;
	uint32_t primitiveCount;
};
#pragma pack(pop)

struct MeshBvh {
	std::vector<BvhNode> m_nodes;
	std::vector<float> m_triangleData;
};

struct BvhRayResult {
	bool hitFound = false;
	size_t primtiviveId = 0;
	uint64_t objectId = 0;
	float distanceToHit = std::numeric_limits<float>::max();
};