#pragma once

#include "Common/Types.h"
#include "Types/VertexArray.h"
#include "RenderList.h"
#include "Renderer/Types/Model.h"
#include "Renderer/Types/Framebuffer.h"
#include "Scene/Components.h"
#include "Scene/Scene.h"
#include "Math/Math.h"

namespace Faint::Renderer {

	inline Ref<Mesh> QuadMesh;
	inline Ref<Mesh> CubeMesh;
	inline Ref<Mesh> SphereMesh;
	inline Ref<FaintUI::Font> m_DefaultFont;

	inline std::vector<Vertex> g_lines;

	void Init();

	void SetViewport(float x, float y, float width, float height);
	void BeginScene();
	void EndScene();

	void SubmitMesh(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityID = -1);
	void Flush(Shader* shader, bool depthOnly = false);

	void OnWindowResize(uint32_t width, uint32_t height);
	
	// Render
	void DrawQuad(Matrix4 transform = Matrix4());
	void DrawCube(Matrix4 transform = Matrix4());

	RendererAPI GetAPI();
}