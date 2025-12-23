#pragma once

#include <memory>

#include "AComponent.h"
#include <Scripting/ScriptEngine.h>

namespace Moon {
	
	class Entity;

	class Behaviour : public AComponent {
	public:
		Behaviour(Entity& p_owner, const std::string& p_name, const std::string& p_path = "");

		~Behaviour();

		virtual std::string GetName() override;

		void SetScript(std::unique_ptr<Scripting::Script>&& p_script);

		std::optional<std::reference_wrapper<Scripting::Script>> GetScript();

		void RemoveScript();

		virtual void OnAwake() override;

		virtual void OnStart() override;

		virtual void OnEnable() override;

		virtual void OnUpdate(float p_deltaTime) override;

		virtual void OnCollisionEnter(ColliderComponent& otherBody) override;

		virtual void OnCollisionStay(ColliderComponent& otherBody) override;

		virtual void OnCollisionExit(ColliderComponent& otherBody) override;

		virtual void OnTriggerEnter(ColliderComponent& otherBody) override;

		virtual void OnTriggerStay(ColliderComponent& otherBody) override;

		virtual void OnTriggerExit(ColliderComponent& otherBody) override;

		const std::string name;
		std::string path;

	private:
		std::unique_ptr<Scripting::Script> m_script;
	};
}