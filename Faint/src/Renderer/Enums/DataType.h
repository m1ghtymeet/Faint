#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class EDataType : uint8_t {
		BYTE,
		UNSIGNED_BYTE,
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT,
		DOUBLE
	};
}