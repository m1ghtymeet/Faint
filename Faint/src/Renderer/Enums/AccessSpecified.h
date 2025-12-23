#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class EAccessSpecified : uint8_t {
		STREAM_DRAW,
		STREAM_READ,
		STREAM_COPY,
		DYNAMIC_DRAW,
		DYNAMIC_READ,
		DYNAMIC_COPY,
		STATIC_DRAW,
		STATIC_READ,
		STATIC_COPY
	};
}