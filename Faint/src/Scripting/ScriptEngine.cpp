#pragma once
#include "ScriptEngine.h"

namespace Faint {
	std::unique_ptr<sol::state> ScriptEngine::luaState;

	sol::object GetComponent(sol::this_state s, const std::string& name) {
		sol::state_view lua(s);
	
		for (Entity& entity : Engine::GetCurrentScene()->GetAllEntities()) {
			if (name == "Transform" && entity.HasComponent<TransformComponent>())
				return sol::make_object(lua, std::ref(entity.GetComponent<TransformComponent>()));
		}
	
		return sol::nil;
	}

	bool HasTransform(Entity& e) { return e.HasComponent<TransformComponent>(); }
	//bool HasRigidBody(Entity& e) { return e.HasComponent<TransformComponent>(); }

	void ScriptEngine::Init() {
		luaState = std::make_unique<sol::state>();
		luaState->open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::os);

		luaState->set_function("createEntity", [](const std::string& name) {
			Entity newEntity = Engine::GetCurrentScene()->CreateEntity(name);
			std::cout << "[Lua] Created Entity '" << name << "' With ID: " << newEntity.GetID() << "\n";
		});

		luaState->set_function("keyDown", [](int key) {
			return Input::KeyDown(key);
		});
		luaState->set_function("keyPress", [](int key) {
			return Input::KeyPressed(key);
		});

		/* ^^^ Binding Entity ^^^ */
		luaState->new_usertype<Entity>("Entity",
			sol::no_constructor,
			/* Methods */
			"GetName", &Entity::GetName,
			//"GetTransform", &Entity::GetComponent<TransformComponent>,
			"HasTransform", &HasTransform
		);

		/* ^^^ Binding Components ^^^ */
		luaState->new_usertype<Component>("Component");
		
		luaState->new_usertype<TransformComponent>("Transform",
			sol::base_classes, sol::bases<Component>(),
			"SetPosition", &TransformComponent::SetLocalPosition
		);
	}
}