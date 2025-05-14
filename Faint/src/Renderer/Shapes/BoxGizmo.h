#pragma once

#include "Core/Base.h"
#include "Renderer/Types/VertexArray.h"
#include <glad/glad.h>

namespace Faint {

	class BoxGizmo {
	public:
		BoxGizmo() = default;
		~BoxGizmo() = default;

		void UpdateShape(float radius, float height);
		void CreateMesh();
		void Bind();
		GLuint GetID() const;

	private:
		std::vector<Vertex> Vertices;
		Ref<VertexArray> _VertexArray;
		Ref<VertexBuffer> _VertexBuffer;
	};
}