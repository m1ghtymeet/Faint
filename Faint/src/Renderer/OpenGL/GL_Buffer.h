#pragma once

#include <Renderer/Common/TBuffer.h>

namespace Moon::Rendering {

	struct OpenGLBufferContext {
		uint32_t id = 0;
		BufferType type = BufferType::UNKNOWN;
		uint64_t allocatedBytes = 0;
	};

	using OpenGLBuffer = TBuffer<OpenGLBufferContext>;
}