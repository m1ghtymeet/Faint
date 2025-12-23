#include "Behaviour.h"
#include <Scene/Entity.h>
#include <Core/GlobalLocator.h>

Moon::Behaviour::Behaviour(Entity& p_owner, const std::string& p_name, const std::string& p_path) :
	name(p_name), path(p_path), AComponent(p_owner) {
	FTSERVICE(Scripting::ScriptEngine).AddBehaviour(*this);
}

Moon::Behaviour::~Behaviour() {
	FTSERVICE(Scripting::ScriptEngine).RemoveBehaviour(*this);
}

std::string Moon::Behaviour::GetName() {
	return "Behaviour";
}

void Moon::Behaviour::SetScript(std::unique_ptr<Scripting::Script>&& p_script) {

	m_script = std::move(p_script);
}

std::optional<std::reference_wrapper<Moon::Scripting::Script>> Moon::Behaviour::GetScript() {
	if (m_script)
		return { *m_script };

	return std::nullopt;
}

void Moon::Behaviour::RemoveScript() {
	m_script.reset();
}

void Moon::Behaviour::OnAwake() {

	FTSERVICE(Scripting::ScriptEngine).OnAwake(*this);
}

void Moon::Behaviour::OnStart() {
	FTSERVICE(Scripting::ScriptEngine).OnStart(*this);
}

void Moon::Behaviour::OnEnable() {
	FTSERVICE(Scripting::ScriptEngine).OnEnable(*this);
}

void Moon::Behaviour::OnUpdate(float p_deltaTime) {
	FTSERVICE(Scripting::ScriptEngine).OnUpdate(*this, p_deltaTime);
}

void Moon::Behaviour::OnCollisionEnter(ColliderComponent& otherBody)
{
}

void Moon::Behaviour::OnCollisionStay(ColliderComponent& otherBody)
{
}

void Moon::Behaviour::OnCollisionExit(ColliderComponent& otherBody)
{
}

void Moon::Behaviour::OnTriggerEnter(ColliderComponent& otherBody) {
	FTSERVICE(Scripting::ScriptEngine).OnTriggerEnter(*this, otherBody);
}

void Moon::Behaviour::OnTriggerStay(ColliderComponent& otherBody) {
	FTSERVICE(Scripting::ScriptEngine).OnTriggerStay(*this, otherBody);
}

void Moon::Behaviour::OnTriggerExit(ColliderComponent& otherBody) {
	FTSERVICE(Scripting::ScriptEngine).OnTriggerExit(*this, otherBody);
}
