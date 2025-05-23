#pragma once
#include "Core/Base.h"
#include "Debug/Log.h"

namespace Faint {

	class NetAPIModule {
	public:
		using MethodMap = std::unordered_map<std::string, void*>;

		virtual const std::string GetModuleName() const = 0;
		virtual void RegisterMethods() = 0;

		const MethodMap& GetMethods() const { return m_Methods; }

	protected:
		void RegisterMethod(const std::string& name, void* methodPtr) {
			m_Methods.emplace(name, methodPtr);
		}

	private:
		MethodMap m_Methods;
	};
}