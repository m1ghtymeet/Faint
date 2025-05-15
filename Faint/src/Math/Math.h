#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include "glm/gtx/matrix_decompose.hpp"
#include <glm/gtx/quaternion.hpp>

namespace Faint {

	// Type definitions
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Quat = glm::quat;
	using Color = glm::vec4;
	using Matrix4 = glm::mat4;
	using Matrix3 = glm::mat3;
#define Rad(degrees) glm::radians(degrees)

	namespace Math {
		bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outTranslation, glm::quat& outRotation, glm::vec3& outScale);
		Quat QuatFromEuler(float x, float y, float z);
	}

	struct ivec2 {
		int x;
		int y;
		ivec2() = default;
		template <typename T>
		ivec2(T x_, T y_) : x(static_cast<int>(x_)), y(static_cast<int>(y_)) {}
		ivec2(const ivec2& other_) : x(other_.x), y(other_.y) {}
		ivec2(int x_, int y_) : x(x_), y(y_) {}
		ivec2 operator+(const ivec2& other) const {
			return ivec2(x + other.x, y + other.y);
		}
		ivec2 operator=(const ivec2& other) const {
			return *this;
		}
	};

	struct vec3f {
		float x = 0;
		float y = 0;
		float z = 0;
		
		vec3f() = default;
		template <typename T>
		vec3f(T x_, T y_, T z_) : x(static_cast<int>(x_)), y(static_cast<int>(y_)), z(static_cast<int>(z_)) {}
		vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
		vec3f(const vec3f& other_) : x(other_.x), y(other_.y), z(other_.z) {}

		vec3f operator-(const vec3f& other) const {
			return vec3f(x - other.x, y - other.y, z - other.z);
		}
		vec3f& operator=(const vec3f& other) {
			if (this != &other) {
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}
		vec3f& operator=(const Vec3& other) {
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}
		vec3f operator+(const vec3f& other) {
			return vec3f(x + other.x, y + other.y, z + other.z);
		}
		vec3f operator*(const vec3f& other) {
			return vec3f(x * other.x, y * other.y, z * other.z);
		}
		vec3f operator*(float scalar) {
			return vec3f(x * scalar, y * scalar, z * scalar);
		}

		vec3f add(const Vec3& left, const Vec3& right) {
			return vec3f(
				left.x + right.x,
				left.y + right.y,
				left.z + right.z);
		}
		vec3f sub(const Vec3& left, const Vec3& right) {
			return vec3f(
				left.x - right.x,
				left.y - right.y,
				left.z - right.z);
		}
		vec3f mul(const Vec3& left, const Vec3& right) {
			return vec3f(
				left.x * right.x,
				left.y * right.y,
				left.z * right.z);
		}
	};

	//inline std::ostream& operator<<(std::ostream& os, vec3 obj) {
	//	os << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";
	//	return os;
	//}

	static const vec3f one(1.0f, 1.0f, 1.0f);
	static const vec3f zero(0.0f, 0.0f, 0.0f);
	static const vec3f forward(0.0f, 0.0f, 1.0f);
	static const vec3f right(1.0f, 0.0f, 0.0f);
	static const vec3f up(0.0f, 1.0f, 0.0f);
}