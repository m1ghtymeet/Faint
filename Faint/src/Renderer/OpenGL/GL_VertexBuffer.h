#pragma once

#include <Renderer/Common/TVertexBuffer.h>
#include "GL_Buffer.h"

namespace Moon::Rendering {

	struct OpenGLVertexBufferContext {};
	using OpenGLVertexBuffer = TVertexBuffer<OpenGLVertexBufferContext, OpenGLBufferContext>;
}