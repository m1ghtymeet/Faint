#include "hzpch.h"
#include "Buffer.h"

#include "../Renderer.h"

#include "OpenGLBuffer.h"

namespace Faint {
    Ref<VertexBuffer> VertexBuffer::Create(std::vector<Vertex>& vertices) {
        
        if (Renderer::GetAPI() == RendererAPI::OPENGL) {
            return CreateRef<OpenGLVertexBuffer>(vertices);
        }
        
        HZ_CORE_ASSERT(false, "Unknown API!");
        return nullptr;
    }
    Ref<IndexBuffer> IndexBuffer::Create(std::vector<uint32_t>& indices) {

        if (Renderer::GetAPI() == RendererAPI::OPENGL) {
            return CreateRef<OpenGLIndexBuffer>(indices);
        }

        HZ_CORE_ASSERT(false, "Unknowm API!");
        return nullptr;
    }
}
