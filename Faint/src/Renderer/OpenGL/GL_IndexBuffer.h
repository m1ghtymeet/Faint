#pragma once

#include <Renderer/Common/TIndexBuffer.h>
#include "GL_Buffer.h"

namespace Moon::Rendering {

	struct OpenGLIndexBufferContext {};
	using OpenGLIndexBuffer = TIndexBuffer<OpenGLIndexBufferContext, OpenGLBufferContext>;
}