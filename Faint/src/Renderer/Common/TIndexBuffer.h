#pragma once

#include <Renderer/Common/TBuffer.h>

namespace Moon::Rendering {
	template<
		class IndexBufferContext,
		class BufferContext>
	class TIndexBuffer : public TBuffer<BufferContext> {
	public:
		TIndexBuffer();

	private:
		uint32_t m_count = 0;
		IndexBufferContext m_context;
	};
}