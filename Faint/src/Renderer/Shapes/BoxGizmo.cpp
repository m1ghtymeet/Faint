#include "BoxGizmo.h"

#include "Math/Math.h"

namespace Faint {

	void BoxGizmo::UpdateShape(float radius, float height)
	{
		CreateMesh();
	}

	void BoxGizmo::CreateMesh()
	{
		std::vector<Vertex> mBoxVertices =
		{
			Vertex{Vec3(-1.f, -1.f, -1.f)},
			Vertex{Vec3(1.0f, -1.f, -1.f)},
			Vertex{Vec3(-1.f, -1.f, 1.f)},
			Vertex{Vec3(1.0f, -1.f, 1.f)},
			Vertex{Vec3(-1.f, 1.f, -1.f)},
			Vertex{Vec3(1.0f, 1.f, -1.f)},
			Vertex{Vec3(-1.f, 1.f, 1.f)},
			Vertex{Vec3(1.0f, 1.f, 1.f)},

			Vertex{Vec3(-1.f, -1.f, -1.f)},
			Vertex{Vec3(-1.0f, -1.f, 1.f)},
			Vertex{Vec3(1.f, -1.f, -1.f)},
			Vertex{Vec3(1.0f, -1.f, 1.f)},
			Vertex{Vec3(-1.f, 1.f, -1.f)},
			Vertex{Vec3(-1.0f, 1.f, 1.f)},
			Vertex{Vec3(1.f, 1.f, -1.f)},
			Vertex{Vec3(1.0f, 1.f, 1.f)},

			Vertex{Vec3(-1.0f, -1.0f, -1.f)},
			Vertex{Vec3(-1.f, 1.0f, -1.f)},
			Vertex{Vec3(1.0f, -1.0f, -1.f)},
			Vertex{Vec3(1.f, 1.0f, -1.f)},
			Vertex{Vec3(-1.0f, -1.0f, 1.f)},
			Vertex{Vec3(-1.f, 1.0f, 1.f)},
			Vertex{Vec3(1.0f, -1.0f, 1.f)},
			Vertex{Vec3(1.0f, 1.0f, 1.f)}
		};

		Vertices = mBoxVertices;

		_VertexArray = VertexArray::Create();
		_VertexArray->Bind();

		_VertexBuffer = VertexBuffer::Create(Vertices);

		BufferLayout layout = {
			{ ShaderDataType::FLOAT3, "aPos" },
		};

		_VertexBuffer->SetLayout(layout);
		_VertexArray->AddVertexBuffer(_VertexBuffer);
	}

	void BoxGizmo::Bind()
	{
		_VertexArray->Bind();
	}

	GLuint BoxGizmo::GetID() const
	{
		return _VertexArray->GetID();
	}
}
