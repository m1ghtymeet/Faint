#pragma once

#include <memory>
#include "Core/Base.h"
#include "Renderer/Types/Buffer.h"
#include <glad/glad.h>

namespace Faint {

	class VertexArray {
	public:
		virtual ~VertexArray();

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
		virtual GLuint GetID() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();
	};
}