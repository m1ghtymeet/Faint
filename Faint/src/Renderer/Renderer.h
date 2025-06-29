#pragma once

#include "Common/Types.h"
#include "RenderList.h"
#include "OpenGL/Types/Mesh.h"
#include "Scene/Scene.h"
#include "Math/Math.h"

namespace Faint::Renderer {

	inline Ref<Mesh> QuadMesh;
	inline Ref<Mesh> CubeMesh;
	inline Ref<Mesh> SphereMesh;

	inline std::vector<Vertex> g_lines;

	void SetAPI(RendererAPI  api);
	RendererAPI GetAPI();

	void Init();

	void BeginScene();
	void EndScene();

	//void SubmitMesh(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityID = -1);
	//void Flush(Shader* shader, bool depthOnly = false);

	// Render
	void DrawQuad(Matrix4 transform = Matrix4());
	void DrawCube(Matrix4 transform = Matrix4());
}