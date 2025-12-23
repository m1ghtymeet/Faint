#pragma once

#include <cstdint>

namespace Moon::Rendering {

	enum class EFramebufferAttachment : uint8_t {
		COLOR = 0,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL
	};
}