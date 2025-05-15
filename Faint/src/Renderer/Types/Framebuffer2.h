#pragma once

#include "Core/Base.h"
#include "glm/glm.hpp"
#include "Renderer/Types/Texture.h"

namespace Faint {

	class Framebuffer2 {
	private:
		unsigned int m_FramebufferID;
		unsigned int m_RenderBuffer;

		Vec2 m_Size;
		bool ResizeQueued = false;

		std::map<int, Ref<Texture>> m_Textures;
		Ref<Texture> m_Texture;
		bool m_HasRenderBuffer = false;

	public:
		Framebuffer2(bool hasRenderBuffer, Vec2 size);
		~Framebuffer2();

		// 0x8CE0 = color attachment 0.
		// TODO: Remove blackbox crap
		Ref<Texture> GetTexture(GLenum attachment = 0x8CE0) { return m_Textures[(int)attachment]; }
		void SetTexture(Ref<Texture> texture, GLenum attachment = 0x8CE0);

		void Clear();
		void Bind();
		void Unbind();
		void QueueResize(Vec2 size);
		Vec2 GetSize() const { return m_Size; }
		void SetSize(const Vec2& size) { m_Size = size; }
		void UpdateSize(Vec2 size);

		uint32_t GetRenderID() const { return m_FramebufferID; }
		void ClearAttachment(uint32_t attachment, int value);
		int ReadPixel(uint32_t attachment, const Vec2 coords);
		float ReadDepth(const Vec2& coords);
		void SetDrawBuffer(GLenum draw);
		void SetReadBuffer(GLenum read);
	};
}