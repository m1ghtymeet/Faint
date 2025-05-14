#pragma once

#include "Core/Base.h"
#include "AssetManagment/Serializable.h"
#include "Component.h"

namespace Faint {

	class AudioComponent : public Component {
	public:
		std::string FilePath;

		bool IsPlaying = false;
		bool Loop = false;

		float Volume = 1.0f;
		float Pan = 0.0f;
		float PlaybackSpeed = 1.0f;

		bool Spatialized = false;
		float MinDistance = 1.0f;
		float MaxDistance = 10.0f;
		float AttenuationFactor = 1.0f;

		json Serialize() {
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(FilePath);
			SERIALIZE_VAL(IsPlaying);
			SERIALIZE_VAL(Volume);
			SERIALIZE_VAL(Pan);
			SERIALIZE_VAL(PlaybackSpeed);
			SERIALIZE_VAL(Spatialized);
			SERIALIZE_VAL(MinDistance);
			SERIALIZE_VAL(MaxDistance);
			SERIALIZE_VAL(AttenuationFactor);
			SERIALIZE_VAL(Loop);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) {
			DESERIALIZE_VAL(FilePath);
			DESERIALIZE_VAL(Volume);
			DESERIALIZE_VAL(IsPlaying);
			DESERIALIZE_VAL(Pan);
			DESERIALIZE_VAL(PlaybackSpeed);
			DESERIALIZE_VAL(Spatialized);
			DESERIALIZE_VAL(MinDistance);
			DESERIALIZE_VAL(MaxDistance);
			DESERIALIZE_VAL(AttenuationFactor);
			DESERIALIZE_VAL(Loop);
			return true;
		}
	};
}