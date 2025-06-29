#pragma once

#include <vector>
#include "Renderer/Types/VertexArray.h"

#include <glad/glad.h>

namespace Faint {

	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override;
		virtual GLuint GetID() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override;

	private:
		GLuint m_ID;
		GLuint m_vertexBufferIndex = 0;
		std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
		Ref<IndexBuffer> m_indexBuffers;
	};
}