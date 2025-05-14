#pragma once
#include "Core/Base.h"
#include "Math/Math.h"
#include "Math/AABB.h"
#include "Scene/Scene.h"
#include "Physics/Physics.h"
#include "Types/FrameBuffer.h"
#include "Types/Framebuffer2.h"

#include "Renderer/Shader.h"
#include "Renderer/Types/Mesh.h"
#include "Renderer/Shapes/BoxGizmo.h"

namespace Faint {
	namespace SceneRenderer {
		void Init();
		void Cleanup();

		void Update(const Time time);

		void BeginRenderScene(const Matrix4& projection, const Matrix4& view, const Vec3& camPos);
		void RenderScene(Scene& scene, Framebuffer2& framebuffer, bool renderUI = true);
	
		FrameBuffer* GetFrameBuffer(const std::string& name);
		Framebuffer2* GetShadingBuffer();

		//BoxGizmo& GetBoxGizmo() const { return *m_BoxGizmo; }

		void DrawDebugLine(const Vec3& start, const Vec3& end, const Color& color = Color(1, 0, 0, 1), float life = 0.0f, float width = 1.0f);
		void DrawDebugShape(const Vec3& position, const Quat& rotation, PxShape* shape, const Color& color = Color(1, 0, 0, 1), float life = 0.0f, float width = 1.0f);

		void GBufferPass(Scene& scene);
		void ShadingPass(Scene& scene, Ref<Texture> previousFrame);
		void ShadowPass(Scene& scene);
		void SkyboxPass();
		void OutlinePass(Scene& scene);
		void DebugPass(Scene& scene);

		inline int m_hoveredEntityID = 0;
	}
}