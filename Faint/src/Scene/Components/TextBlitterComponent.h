#pragma once

#include "Core/Base.h"
#include "AssetManagment/Serializable.h"
#include "Component.h"

namespace Faint {

	class TextBlitterComponent : public Component {
	public:
		
		std::string Text;
		std::string FontName = "StandardFont";
		int LocationX;
		int LocationY;
		float Scale = 1.0f;

		json Serialize() {
			BEGIN_SERIALIZE();

			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {

			return true;
		}
	};
}