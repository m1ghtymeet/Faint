#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class ETextureType : uint8_t {
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE_MAP
	};
}