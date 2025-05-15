#pragma once

#include <glm/glm.hpp>

namespace Faint {

	enum class RendererEnum {
		INT, UINT, BYTE, UBYTE,
		FLOAT, UFLOAT,
		ARRAY_BUFFER,
		ELEMENT_ARRAY_BUFFER,
		TRIANGLES, LINES,
		DEPTH_ATTACHMENT, COLOR_ATTACHMNET0, COLOR_ATTACHMENT1,
		DEPTH_TEST, FACE_CULL, BLENDING,
		STATIC_DRAW, DYNAMIC_DRAW, STREAM_DRAW
	};

	class RenderCommand {
	public:
		static void Clear();
		static void SetClearColor(const glm::vec4& color);

		static void Enable(const RendererEnum enumType);
		static void Disable(const RendererEnum enumType);
		static void GenBuffer(const RendererEnum bufferType, unsigned int& bufferID);
		static void DeleteBuffer(unsigned int bufferID);

		static void BindVertexArray(const unsigned int& rendererID);
		static void VertexAttribPointer(const unsigned int index, const int size, const RendererEnum type, bool normalized, int stride, const void* pointer);
		static void DrawMultiElements(const RendererEnum mode, const int count, const RendererEnum type, const void* const* indices, unsigned int drawCount);
		static void DrawElements(const RendererEnum mode, const uint32_t count, const RendererEnum type, const void* indices);
		static void DrawArrays(int first, int count);

		static void DrawLines(int first, int count);
	};
}