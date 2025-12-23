#pragma once
#include <optional>
#include <Scene/Camera.h>
#include <Renderer/OpenGL/GL_frameBuffer.h>

namespace Moon::Rendering::Data {

	//enum class RendererOrder { SHADED, BASECOLOR, NORMAL, RMA, WORLDSPACE };

	struct FrameDescriptor {
	public:
		uint16_t renderWidth = 0;
		uint16_t renderHeight = 0;
		std::optional<Camera> camera;
		FrameBuffer* outputBuffer = nullptr;

		bool IsValid() const {
			return camera.has_value();
		}
	};
}