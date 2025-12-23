#pragma once

#include "TVertexBuffer.h"
#include "TIndexBuffer.h"
#include "Types.h"

namespace Moon::Rendering {
	template<
		class VertexArrayContext,
		class VertexBufferContext,
		class IndexBufferContext,
		class BufferContext>
	class TVertexArray final {
	public:
		using VertexBuffer = TVertexBuffer<VertexBufferContext, BufferContext>;
		using IndexBuffer = TIndexBuffer<IndexBufferContext, BufferContext>;

		TVertexArray();
		~TVertexArray();

		void ApplyLayout(VertexAttributeLayout p_attributes, VertexBuffer& vb, IndexBuffer& ib);
		void ResetLayout();

		void Bind() const;
		void Unbind() const;

		bool IsValid() const;

		uint32_t GetID() const;

	private:
		VertexArrayContext m_context;
	};
}