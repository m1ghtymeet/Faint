#pragma once

#include "Core/Base.h"
#include "AssetManagment/Serializable.h"
#include "Component.h"

namespace Faint {

	class TextComponent : public Component {
	public:
		
		std::string TextString;
		Ref<FaintUI::Font> FontAsset;
		Vec4 Color{ 1.0f };
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;

		json Serialize() {
			BEGIN_SERIALIZE();

			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {

			return true;
		}
	};
}