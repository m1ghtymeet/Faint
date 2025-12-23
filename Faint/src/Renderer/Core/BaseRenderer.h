#pragma once

#include <atomic>

#include <glm/glm.hpp>
#include <Renderer/Data/FrameDescriptor.h>

namespace Moon::Rendering {

	class IRenderer {
	public:
		virtual void BeginFrame(const Data::FrameDescriptor& p_frameDescriptor) = 0;
		virtual void DrawFrame() = 0;
		virtual void EndFrame() = 0;
	};

	class BaseRenderer : public IRenderer {
	public:
		BaseRenderer();

		virtual ~BaseRenderer() = default;

		virtual void BeginFrame(const Data::FrameDescriptor& p_frameData);

		virtual void EndFrame();

		const Data::FrameDescriptor& GetFrameDescriptor() const;

		bool IsDrawing() const;

		void Clear(bool p_colorBuffer, bool p_depthBuffer, bool p_stencilBuffer, glm::vec4& p_color);

	protected:
		Data::FrameDescriptor m_frameDescriptor;
		bool m_isDrawing;

	private:
		static std::atomic_bool s_isDrawing;
	};
}