#pragma once
#include <memory>
#include <string>
#include <any>
#include <Coral/HostInstance.hpp>
#include <Coral/Type.hpp>
#include <Coral/ManagedObject.hpp>

namespace Moon { class Entity; }
namespace Moon::Scripting {
	class CoralScriptEngine;

	class CoralScript {
	public:
		CoralScript(const std::string& assemblyPath, const std::string& className);
		virtual ~CoralScript() = default;

		void SetOwner(Entity& p_owner);
		bool IsValid() const;

		void Call(const std::string& methodName);

		template<typename... Args>
		void Call(const std::string& methodName, Args&&... args);

		template<typename T, typename... Args>
		T CallWithReturn(const std::string& methodName, Args&&... args);

		Entity* GetOwner() const { return m_owner; }

	private:
		friend class CoralScriptEngine;

		Coral::ManagedObject m_instance;
		bool m_isValid = false;
		Entity* m_owner = nullptr;
	};
}

#include "CSharpScript.inl"