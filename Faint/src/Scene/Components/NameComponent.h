#pragma once

#include "Common/UUID.h"
#include "AssetManagment/Serializable.h"

namespace Faint {

	class NameComponent {
	public:
		UUID id;
		std::string name = "Entity";
		bool isPrefab = false;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(std::string name) {
			this->name = name;
		}

		json Serialize() {
			BEGIN_SERIALIZE();
			END_SERIALIZE();
		}
	};
}