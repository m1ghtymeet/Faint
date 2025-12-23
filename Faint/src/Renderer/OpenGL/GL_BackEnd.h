#pragma once
#include <Renderer/Types/Vertex.h>
#include <Renderer/Types/WeightedVertex.h>
#include <vector>

namespace Moon::Rendering {
	namespace OpenGLBackEnd {
		void InitGlobalBuffers();
		void Cleanup();

		void UpdateVertexBufferData(const Vertex* data, size_t count);
		void UpdateIndexBufferData(const uint32_t* data, size_t count);

		void UpdateWightedVertexData(const Rendering::WeightedVertex* data, size_t count);
		void UpdateWightedIndexData(const uint32_t* data, size_t count);

		uint32_t GetVertexDataVAO();
		//uint32_t GetVertexDataVBO();
		//uint32_t GetVertexDataEBO();

		uint32_t GetWeightedVertexDataVAO();
		uint32_t GetWeightedVertexDataVBO();
		uint32_t GetWeightedVertexDataEBO();
	}
}