#include "CSharpScript.h"
#include <Coral/Array.hpp>
#include <memory>

template<typename ...Args>
void Moon::Scripting::CoralScript::Call(const std::string& methodName, Args&& ...args) {
	if (!m_isValid) return;
	try {
		Coral::Array argsArray{ std::forward<Args>(args)... };
		m_instance.InvokeMethod(methodName, argsArray);
	}
	catch (const std::exception& ex) {
		// TODO: Error log
	}
}

template<typename T, typename ...Args>
T Moon::Scripting::CoralScript::CallWithReturn(const std::string& methodName, Args&&... args) {
	if (!m_isValid) return T{};
	try {
		Coral::Array argsArray{ std::forward<Args>(args)... };
		if constexpr (std::is_void_v<T>)
			m_instance.InvokeMethod(methodName, argsArray);
		else
			return m_instance.InvokeMethod<T>(methodName, std::forward<Args>(args));
	}
	catch (const std::exception& ex) {
		// TODO: Error log
		// [Coral] CoralScript::CallWithReturn failed: {} | {}
		return T{};
	}
}