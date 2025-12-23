#pragma once
#include <Renderer/Common/TBuffer.h>

namespace Moon::Rendering {
	template<
		class VertexBufferContext,
		class BufferContext>
	class TVertexBuffer final : public TBuffer<BufferContext> {
	public:
		TVertexBuffer();

	private:
		VertexBufferContext m_context;
	};
}