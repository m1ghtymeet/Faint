#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class BufferType : uint8_t {
		VERTEX,
		INDEX,
		UNKNOWN
	};
}