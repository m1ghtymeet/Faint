#pragma once

#include <Renderer/Features/ARenderFeature.h>
#include <Renderer/OpenGL/GL_DebugMesh.hpp>

namespace Moon {
	struct AABB;
}

namespace Moon::Rendering {
	
	class DebugLineRenderFeature : public ARenderFeature {
	public:
		DebugLineRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy = FeatureExecutionPolicy::WHITELIST_ONLY);

		std::vector<DebugVertex> GetLines();

		OpenGLDebugMesh GetLinesMesh();

		void AddLine(glm::vec3 start, glm::vec3 end, glm::vec3 color = { 1, 1, 1 });
		void DrawBox(const glm::vec3& p_position, const glm::quat& p_rotation, const glm::vec3& p_size, const glm::vec3& p_color, float p_lineWidth = 1.0f, bool p_depthTest = true);
		void DrawSphere(const glm::vec3& p_position, const glm::quat& p_rotation, float p_radius, const glm::vec3& p_color, float p_lineWidth = 1.0f, bool p_depthTest = false);
		void DrawCapsule(const glm::vec3& p_position, const glm::quat& p_rotation, float p_radius, float p_height, const glm::vec3& p_color, float p_lineWidth = 1.0f, bool p_depthTest = true);
		void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform);

	protected:
		virtual void OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor) override;

	private:
		std::vector<DebugVertex> m_lines;
		OpenGLDebugMesh m_debugLinesMesh;
	};
}