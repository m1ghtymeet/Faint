#include "BaseRenderer.h"
#include <Debug/Assertion.h>

#include <glad/glad.h>

std::atomic_bool Moon::Rendering::BaseRenderer::s_isDrawing{ false };
Moon::FrameBuffer* finalImageFrameBuffer;

Moon::Rendering::BaseRenderer::BaseRenderer() :
	m_isDrawing(false) {

	finalImageFrameBuffer = new FrameBuffer("FinalImage", 1600, 900);
	finalImageFrameBuffer->CreateAttachment("Color", GL_RGBA16F);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Moon::Rendering::BaseRenderer::BeginFrame(const Data::FrameDescriptor& p_frameData) {
	FT_CORE_ASSERT(!s_isDrawing, "Cannot call BeginFrame() when previous frame hansn't finished.");
	FT_CORE_ASSERT(p_frameData.IsValid(), "Invalid FrameDescriptor!");

	m_frameDescriptor = p_frameData;
	m_frameDescriptor.outputBuffer = finalImageFrameBuffer;

	glViewport(0, 0, p_frameData.renderWidth, p_frameData.renderHeight);
	//glClearColor(0, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	m_isDrawing = true;
	s_isDrawing.store(true);
}

void Moon::Rendering::BaseRenderer::EndFrame() {
	FT_CORE_ASSERT(m_isDrawing, "Cannot call EndFrame() before calling BeginFrame()");

	m_isDrawing = false;
	s_isDrawing.store(false);
}

const Moon::Rendering::Data::FrameDescriptor& Moon::Rendering::BaseRenderer::GetFrameDescriptor() const {
	return m_frameDescriptor;
}
