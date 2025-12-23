#pragma once

#include <math.h>

struct Vector3 {
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };

	// Constructors
	constexpr Vector3() noexcept = default;
	constexpr Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {}
	constexpr Vector3(float p_x, float p_y, float p_z) : x(p_x), y(p_y), z(p_z) {}

	constexpr Vector3(const Vector3&) noexcept = default;
	constexpr Vector3(Vector3&&) noexcept = default;
	constexpr Vector3& operator=(const Vector3&) noexcept = default;
	constexpr Vector3& operator=(Vector3&&) noexcept = default;

	// Unary operators
	constexpr Vector3 operator-() const noexcept { return { -x, -y, -z }; }

	// Arithmetic operators
	constexpr Vector3 operator+(const Vector3& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z };
	}
	constexpr Vector3 operator-(const Vector3& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z };
	}
	constexpr Vector3 operator*(const Vector3& other) const noexcept {
		return { x * other.x, y * other.y, z * other.z };
	}
	constexpr Vector3 operator/(const Vector3& other) const noexcept {
		return { x / other.x, y / other.y, z / other.z };
	}

	// Scalar addition/multiplication/division
	constexpr Vector3 operator+(float scalar) const noexcept {
		return { x + scalar, y + scalar, z + scalar };
	}
	constexpr Vector3 operator-(float scalar) const noexcept {
		return { x - scalar, y - scalar, z - scalar };
	}
	constexpr Vector3 operator*(float scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar };
	}
	constexpr Vector3 operator/(float scalar) const noexcept {
		return { x / scalar, y / scalar, z / scalar };
	}

	// ==---== Math functions ==---==
	float Length() const noexcept { return sqrt(x * x + y * y + z * z); }
	float LengthSquared() const noexcept { return x * x + y * y + z * z; }

	Vector3 Normalized() const noexcept {
		float len = Length();
		if (len > 0.00001f) return { x / len, y / len, z / len };
		return { 0, 0, 0 };
	}

	void Normalize() noexcept {
		float len = Length();
		if (len > 0.00001f) { x /= len, y /= len, z /= len; };
	}

	static float Dot(const Vector3& a, const Vector3& b) noexcept {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector3 Cross(const Vector3& a, const Vector3& b) noexcept {
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
		};
	}
};

struct Matrix4 {
	float m[16]; // column-major

	Matrix4() {
		for (int i = 0; i < 16; i++) m[i] = 0.0f;
		m[0] = m[5] = m[10] = m[15] = 1.0f; // identity
	}

	static Matrix4 Identity() {
		return Matrix4();
	}

	static Matrix4 Translation(const Vector3& v) {
		Matrix4 mat = Identity();
		mat.m[12] = v.x;
		mat.m[13] = v.y;
		mat.m[14] = v.z;
		return mat;
	}
};