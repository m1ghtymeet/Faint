#include "CSharpScript.h"
#include <Debug/Log.h>
#include <Scene/Entity.h>
#include <Coral/Array.hpp>

Moon::Scripting::CoralScript::CoralScript(const std::string& assemblyPath, const std::string& className) {
}

void Moon::Scripting::CoralScript::SetOwner(Entity& p_owner) {
	m_owner = &p_owner;
	if (m_isValid) {
		try {
			/*m_instance.SetFieldValue("Entity", Coral::ManagedObject::)*/
		}
		catch (...) { /* ignore */ }
	}
}

bool Moon::Scripting::CoralScript::IsValid() const {
	return m_isValid;
}

void Moon::Scripting::CoralScript::Call(const std::string& name) {
	if (!m_isValid) return;
	try {
		m_instance.InvokeMethod(name, Coral::Array<void*>());
	}
	catch (const std::exception& ex) {
		HZ_CORE_ERROR("[Coral] Call failed: {} | {}", name, ex.what());
	}
}

