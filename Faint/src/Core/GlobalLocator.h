#pragma once

#include <any>
#include <unordered_map>

#define FTSERVICE(Type) Moon::GlobalLocator::Get<Type>()

namespace Moon {

	class GlobalLocator {
	public:

		template<typename T>
		static void Provide(T& p_service) {
			__SERVICES[typeid(T).hash_code()] = std::any(&p_service);
		}

		template<typename T>
		static T& Get() {
			return *std::any_cast<T*>(__SERVICES[typeid(T).hash_code()]);
		}

	private:
		static std::unordered_map<size_t, std::any> __SERVICES;
	};
}