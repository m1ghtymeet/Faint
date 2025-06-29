#pragma once
#include "Core/Base.h"
#include "Math/Math.h"
#include "Math/AABB.h"
#include "Scene/Scene.h"
#include "Renderer/OpenGL/Types/GL_FrameBuffer.h"
#include "Renderer/OpenGL/Types/GL_SSBO.hpp"

#include "Renderer/Shader.h"
#include "Renderer/OpenGL/Types/Mesh.h"

namespace Faint {
	namespace SceneRenderer {
		void Init();
		void LoadShaders();
		void Cleanup();

		void Update(const Time time);
		void UpdateSSBOS();

		// SSBOs
		void CreateSSBO(const std::string& name, float size, GLbitfield flags);
		OpenGLSSBO* GetSSBO(const std::string& name);

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos);
		void RenderScene(Scene& scene, bool renderUI = true);

		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene);
		void ShadowPass(Scene& scene);
		void SkyboxPass();
		void DebugPass(Scene& scene);
		void OutlinePass(Scene& scene);
		void OutlinePassEdgeDetection(Scene& scene);

		// Debug
		void UpdateDebugMesh();
		void DrawLine(Vec3 begin, Vec3 end, Vec3 color, bool obeyDepth = false);
		void DrawAABB(const AABB& aabb, const glm::vec3& color);
		void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform);

		inline std::vector<DebugVertex> g_points;
		inline std::vector<DebugVertex> g_lines;

		Shader* GetShader(const std::string& name);
		FrameBuffer* GetFrameBuffer(const std::string& name);

		// Util
		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, GLbitfield mask, GLenum filter);
		void BlitToDefaultFrameBuffer(FrameBuffer* srcFrameBuffer, const char* srcName, glm::vec4 srcRect, glm::vec4 dstRect, GLbitfield mask, GLenum filter);
		void BlitFrameBuffer(FrameBuffer* srcFrameBuffer, FrameBuffer* dstFrameBuffer, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter);

		inline int m_hoveredEntityID = 0;
		inline int m_selectedEntityID = 0;
	}
}