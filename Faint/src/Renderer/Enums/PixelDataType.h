#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class EPixelDataType : uint8_t {
		BYTE,
		UNSIGNED_BYTE,
		BITMAP,
		SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT
	};
}