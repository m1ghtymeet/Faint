#pragma once
#include "ScriptEngine.h"

namespace Faint {
	std::unique_ptr<sol::state> ScriptEngine::luaState;

	sol::object GetComponent(sol::this_state s, const std::string& name) {
		sol::state_view lua(s);
	
		for (Entity* entity : Engine::GetCurrentScene()->GetAllEntities()) {
			if (name == "Transform" && entity->GetComponent<TransformComponent>())
				return sol::make_object(lua, std::ref(*entity->GetComponent<TransformComponent>()));
		}
	
		return sol::nil;
	}

	bool HasTransform(Entity& e) { return e.GetComponent<TransformComponent>(); }
	//bool HasRigidBody(Entity& e) { return e.HasComponent<TransformComponent>(); }
	TransformComponent GetTransform(Entity& e) { return *e.GetComponent<TransformComponent>(); }

	void ScriptEngine::Init() {
		luaState = std::make_unique<sol::state>();
		luaState->open_libraries(
			sol::lib::base,
			sol::lib::package,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table,
			sol::lib::debug
		);

		BindCoreTypes();
		BindInput();
		BindEntity();
		BindComponents();
	}
	void ScriptEngine::Shutdown() {
		luaState.reset();
	}
	void ScriptEngine::BindCoreTypes() {

	}
	void ScriptEngine::BindInput() {
		luaState->set_function("keyDown", [](int key) {
			return Input::KeyDown(key);
			});
		luaState->set_function("keyPress", [](int key) {
			return Input::KeyPressed(key);
			});
	}
	void ScriptEngine::BindEntity() {
		luaState->new_usertype<Entity>("Entity",
			sol::no_constructor,
			/* Methods */
			"getID", &Entity::GetID,
			"getName", &Entity::GetName,
			"addComponent", [](Entity& self, const std::string& type) {

			},
			"getComponent", [](Entity& self, const std::string& type) -> sol::object {
				return sol::nil;
			},
			"hasComponent", [](Entity& self, const std::string& type) {
				return false;
			}
		);
	}
	void ScriptEngine::BindComponents() {

	}
}