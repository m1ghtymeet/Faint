
#include <sol/sol.hpp>
#include <Scripting/ScriptEngine.h>

#include <Core/GlobalLocator.h>
#include <Math/Types.h>
#include <Physics/PhysicsEngine.h>

void BindLuaPhysics(sol::state& p_state) {

	using namespace Moon::Physics;

	p_state.new_usertype<PhysicsActor>("PhysicsActor",
		"Name", sol::property(&PhysicsActor::m_name),
		"owner", &PhysicsActor::m_entity,
		"userdata", sol::property([](PhysicsActor& self) -> Moon::Entity& {
			if (self.GetInlineUserData().has_value()) {
				if (auto comp = std::any_cast<Moon::AComponent*>(self.GetInlineUserData()))
					return comp->owner;
			}
			throw std::runtime_error("UserData is empty or not an AComponent");
		})
	);

	p_state.create_named_table("Physics",
		"Raycast", [&](Vector3 origin, Vector3 direction, float distance, uint32_t flags) {
			Moon::Physics::PhysXRayResult hit = FTSERVICE(PhysicsEngine).CastPhysXRay(
				{ origin.x, origin.y, origin.z },
				{ direction.x, direction.y, direction.z },
				distance,
				flags,
				false,
				true
			);
			sol::table hitInfo = p_state.create_table();
			hitInfo["found"] = hit.found;
			hitInfo["position"] = Vector3{ hit.position.x, hit.position.y, hit.position.z };
			hitInfo["normal"] = Vector3{ hit.normal.x, hit.normal.y, hit.normal.z };
			hitInfo["name"] = hit.name;
			hitInfo["distance"] = hit.distance;
			if (hit.actor) {
				hitInfo["actor"] = hit.actor;
			}
			return hitInfo;
		}
	);
}