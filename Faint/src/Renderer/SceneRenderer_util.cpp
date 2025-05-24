#include "SceneRenderer.h"
#include "Core/Window.h"

namespace Faint {
	namespace SceneRenderer {
		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			if (srcAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(GL_BACK);
				glm::vec4 srcRect;
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.z = (float)srcFrameBuffer->GetWidth();
				srcRect.w = (float)srcFrameBuffer->GetHeight();
				glm::vec4 dstRect;
				dstRect.x = 0;
				dstRect.y = 0;
				dstRect.z = Window::Get()->GetWidth();
				dstRect.w = Window::Get()->GetHeight();
				glBlitFramebuffer(0, 0, (GLint)srcFrameBuffer->GetWidth(), (GLint)srcFrameBuffer->GetHeight(), 0, 0, (GLint)Window::Get()->GetWidth(), (GLint)Window::Get()->GetHeight(), mask, filter);
			}
		}
		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, glm::vec4 srcRect, glm::vec4 dstRect, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			if (srcAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer((GLint)srcRect.x, (GLint)srcRect.y, (GLint)srcRect.z, (GLint)srcRect.w, (GLint)dstRect.x, (GLint)dstRect.y, (GLint)dstRect.z, (GLint)dstRect.w, mask, filter);
			}
		}
		void BlitFrameBuffer(FrameBuffer* srcFrameBuffer, FrameBuffer* dstFrameBuffer, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter) {
			GLint srcAttachmentSlot = srcFrameBuffer->GetColorAttachmentSlotByName(srcName);
			GLint dstAttachmentSlot = dstFrameBuffer->GetColorAttachmentSlotByName(dstName);
			if (srcAttachmentSlot != GL_INVALID_VALUE && dstAttachmentSlot != GL_INVALID_VALUE) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFrameBuffer->GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFrameBuffer->GetHandle());
				glReadBuffer(srcAttachmentSlot);
				glDrawBuffer(dstAttachmentSlot);
				glm::vec4 srcRect;
				srcRect.x = 0;
				srcRect.y = 0;
				srcRect.z = (float)srcFrameBuffer->GetWidth();
				srcRect.w = (float)srcFrameBuffer->GetHeight();
				glm::vec4 dstRect;
				dstRect.x = 0;
				dstRect.y = 0;
				dstRect.z = (float)dstFrameBuffer->GetWidth();
				dstRect.w = (float)dstFrameBuffer->GetHeight();
				glBlitFramebuffer(0, 0, (GLint)srcFrameBuffer->GetWidth(), (GLint)srcFrameBuffer->GetHeight(), 0, 0, (GLint)dstFrameBuffer->GetWidth(), (GLint)dstFrameBuffer->GetHeight(), mask, filter);
			}
		}
	}
}
