
#include <sol/sol.hpp>

#include <Math/Types.h>

float Lerp(float a, float b, float t) {
	return a + t * (b - a);
}

void BindLuaMath(sol::state& p_state) {

	p_state.create_named_table("Math",
		"Lerp", [](float a, float b, float t) -> float { return Lerp(a, b, t); }
	);
	
	p_state.create_named_table("Util",
		"FInterpTo", [](float current, float target, float deltaTime, float interpSpeed) -> float {
			if (interpSpeed <= 0.0f)
				return target;
			float dist = target - current;
			if (dist * dist < (float)9.99999993922529e-9)
				return target;
			float deltaMove = dist * std::max(0.0f, std::min(deltaTime * interpSpeed, 1.0f));
			return current + deltaMove;
		}
	);

	p_state.new_usertype<Vector3>("Vector3",
		sol::constructors<
			Vector3(),
			Vector3(float),
			Vector3(float, float, float)
		>(),

		// Operators
		sol::meta_function::addition, sol::overload(
			sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator+),
			sol::resolve<Vector3(float) const>(&Vector3::operator+),
			[](float s, const Vector3& v) { return v + s; }
		),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator-),
			sol::resolve<Vector3(float) const>(&Vector3::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<Vector3(float) const>(&Vector3::operator*),
			sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator*),
			[](float s, const Vector3& v) { return v * s; }
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<Vector3(float) const>(&Vector3::operator/),
			sol::resolve<Vector3(const Vector3&) const>(&Vector3::operator/),
			[](float s, const Vector3& v) { return v / s; }
		),
		sol::meta_function::unary_minus, sol::resolve<Vector3() const>(&Vector3::operator-),
		
		sol::meta_function::to_string, [](const Vector3& target) { return "(" + std::to_string(target.x) + ", " + std::to_string(target.y) + ", " + std::to_string(target.z) + ")"; },

		// Fields
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z,

		// Methods
		"length", &Vector3::Length,
		"lengthSquared", &Vector3::LengthSquared,
		"normalized", &Vector3::Normalized,
		"normalize", &Vector3::Normalize,
		"dot", &Vector3::Dot,
		"cross", &Vector3::Cross,
		"lerp", [](const Vector3& a, const Vector3& b, float t) -> Vector3 {
			return Vector3(
				Lerp(a.x, b.x, t),
				Lerp(a.y, b.y, t),
				Lerp(a.z, b.z, t)
			);
		}
	);
}