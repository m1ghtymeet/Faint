#include "Framebuffer2.h"
#include "Debug/Log.h"
#include <glad/glad.h>

namespace Faint
{
	Framebuffer2::Framebuffer2(bool hasRenderBuffer, Vec2 size)
	{
		m_Textures = std::map<int, Ref<Texture>>();
		m_Size = size;
		m_HasRenderBuffer = hasRenderBuffer;

		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FramebufferID);

		// Create render buffer and attach to frame buffer.
		if (m_HasRenderBuffer)
		{
			glGenRenderbuffers(1, &m_RenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
		}
		else
		{
			m_RenderBuffer = -1;
		}

		// Unbind
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	Framebuffer2::~Framebuffer2()
	{
		glDeleteFramebuffers(1, &m_FramebufferID);
		glDeleteRenderbuffers(1, &m_RenderBuffer);
	}

	void Framebuffer2::SetTexture(Ref<Texture> texture, GLenum attachment)
	{
		m_Textures[attachment] = texture;
		// Attach texture to the framebuffer.
		Bind();
		texture->AttachToFramebuffer(attachment);

		// Set draw buffer with dynamic amount of render target.
		// Surely, this can be optimized.
		int size = 0;
		std::vector<unsigned int> keys = std::vector<unsigned int>();
		for (auto& s : m_Textures)
		{
			if (s.first == GL_DEPTH_ATTACHMENT)
				continue;

			keys.push_back(s.first);
			size += 1;
		}

		if (size > 0)
			glDrawBuffers(size, &keys[0]);

		Unbind();
	}


	void Framebuffer2::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Framebuffer2::Bind()
	{
		if (ResizeQueued)
			UpdateSize(m_Size);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

		glViewport(0, 0, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
	}

	void Framebuffer2::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer2::QueueResize(Vec2 size)
	{
		if (size == m_Size)
			return;

		ResizeQueued = true;
		m_Size = size;
	}

	void Framebuffer2::UpdateSize(Vec2 size)
	{
		m_Size = size;
		ResizeQueued = false;

		// Delete frame buffer and render buffer.
		glDeleteFramebuffers(1, &m_FramebufferID);
		if (m_HasRenderBuffer)
			glDeleteRenderbuffers(1, &m_RenderBuffer);

		// New FBO and RBO.
		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FramebufferID);

		// Recreate resized texture.
		for (auto& t : m_Textures)
		{
			t.second->Resize(size);
			SetTexture(t.second, t.first);
		}

		// Recreate render buffer
		// TODO: move out render buffer.
		if (m_HasRenderBuffer)
		{
			glGenRenderbuffers(1, &m_RenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
		}

		HZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		// Unbind.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void Framebuffer2::ClearAttachment(uint32_t attachment, int value)
	{
		glClearTexImage(attachment, 0, GL_RED_INTEGER, GL_INT, &value);
	}

	int Framebuffer2::ReadPixel(uint32_t attachment, const Vec2 coords)
	{
		//HZ_CORE_ASSERT(attachment < .size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
		int pixelData;
		glReadPixels((int)coords.x, (int)coords.y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	float Framebuffer2::ReadDepth(const Vec2& coords)
	{
		glReadBuffer(GL_DEPTH_ATTACHMENT);
		float pixelData;
		glReadPixels((int)coords.x, (int)coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixelData);
		return pixelData;
	}

	void Framebuffer2::SetDrawBuffer(GLenum draw)
	{
		Bind();
		//glReadBuffer(draw);
		Unbind();
	}

	void Framebuffer2::SetReadBuffer(GLenum read)
	{
		Bind();
		//glReadBuffer(read);
		Unbind();
	}
}
