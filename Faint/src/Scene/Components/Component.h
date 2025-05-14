#pragma once

#include <entt.hpp>

namespace Faint {

	class Component {
	protected:
		static void InitializeComponentClass() {}

		static void(*GetInitializeComponentClass(void))() {
			return &Component::InitializeComponentClass;
		}

	public:
		static std::string GetName(const entt::meta_type& componentMeta);
	};
}