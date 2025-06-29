#pragma once

#include "Common/Types.h"
#include "Renderer/Types/Buffer.h"

namespace Faint {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(std::vector<Vertex>& vertices);
		~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_layout = layout; }
	private:
		uint32_t m_ID;
		BufferLayout m_layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(std::vector<uint32_t> indices);
		~OpenGLIndexBuffer();

		virtual void Bind() const;
		virtual void UnBind() const;

		virtual uint32_t GetCount() const { return m_count; };
	private:
		uint32_t m_ID;
		uint32_t m_count;
	};
}