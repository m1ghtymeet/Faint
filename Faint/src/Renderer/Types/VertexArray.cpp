#include "hzpch.h"
#include "VertexArray.h"

#include "../Renderer.h"

#include "OpenGLVertexArray.h"

namespace Faint {
	VertexArray::~VertexArray() {

	}
	Ref<VertexArray> VertexArray::Create() {
		if (Renderer::GetAPI() == RendererAPI::OPENGL)
			return CreateRef<OpenGLVertexArray>();

		HZ_CORE_ASSERT(false, "Unknown API!");
		return nullptr;
	}
}
