#pragma once

#include "Core/Base.h"
#include "AssetManagment/Serializable.h"
#include "Component.h"
#include "Common/UUID.h"

namespace Faint {

	class SkyComponent : public Component {
	public:
		UUID Resource = UUID(0);
		std::string SkyResourceFilePath;

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(SkyResourceFilePath);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			DESERIALIZE_VAL(SkyResourceFilePath);
			return true;
		}
	};
}