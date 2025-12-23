#include "Util.h"

#include <glad/glad.h>

namespace Moon::Rendering::Util {
	PipelineState g_pipelineState;
}

void Moon::Rendering::Util::SetPipelineState(PipelineState p_state) {
	const PipelineState& current = g_pipelineState;

	// Only apply changes if the pipeline state has actually changed
	if (p_state._bits == current._bits)
		return;

	// Blending
	if (p_state.blendingEnable != current.blendingEnable) {
		if (p_state.blendingEnable)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	// Face Culling
	if (p_state.cullingEnable != current.cullingEnable) {
		if (p_state.cullingEnable)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	// Depth Test
	if (p_state.depthTestEnable != current.depthTestEnable) {
		if (p_state.depthTestEnable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	// Depth Write (Mask)
	if (p_state.depthWriteEnable != current.depthWriteEnable)
		glDepthMask(p_state.depthWriteEnable ? GL_TRUE : GL_FALSE);

	// Color Write Mask
	if (p_state.colorWriteMask.r != current.colorWriteMask.r ||
		p_state.colorWriteMask.g != current.colorWriteMask.g ||
		p_state.colorWriteMask.b != current.colorWriteMask.b ||
		p_state.colorWriteMask.a != current.colorWriteMask.a) {
		glColorMask(
			p_state.colorWriteMask.r ? GL_TRUE : GL_FALSE,
			p_state.colorWriteMask.g ? GL_TRUE : GL_FALSE,
			p_state.colorWriteMask.b ? GL_TRUE : GL_FALSE,
			p_state.colorWriteMask.a ? GL_TRUE : GL_FALSE
		);
	}

	// Stencil Test
	if (p_state.stencilTestEnable != current.stencilTestEnable) {
		if (p_state.stencilTestEnable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	// Line Width (if supported)
	if (p_state.lineWidthPow2 != current.lineWidthPow2) {
		float lineWidth = 1.0f * (1 << p_state.lineWidthPow2); // 1, 2, 4, 8...
		glLineWidth(lineWidth);
	}

	// Stencil Parameters (if test enabled)
	// Note: These are typically set per-face or globally - adjust as needed
	if (p_state.stencilTestEnable &&
		(p_state.stencilFuncRef != current.stencilFuncRef ||
		 p_state.stencilFuncMask != current.stencilFuncMask ||
		 p_state.stencilWriteMask != current.stencilWriteMask)) {
		glStencilFunc(GL_ALWAYS, p_state.stencilFuncRef, p_state.stencilFuncMask);
		glStencilMask(p_state.stencilWriteMask);
		// You might went separate front/back settings in a more advanced version
	}

	// Finally, update the cached global state
	g_pipelineState = p_state;
}
