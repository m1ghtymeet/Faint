#pragma once

#include <Renderer/Common/TVertexArray.h>
#include "GL_IndexBuffer.h"
#include "GL_VertexBuffer.h"

namespace Moon::Rendering {

	struct OpenGLVertexArrayContext {
		uint32_t id = 0;
		uint32_t attributeCount = 0;
	};

	using OpenGLVertexArray = TVertexArray<
		OpenGLVertexArrayContext,
		OpenGLVertexBufferContext,
		OpenGLIndexBufferContext,
		OpenGLBufferContext
	>;
}