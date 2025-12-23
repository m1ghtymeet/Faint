#include "DebugLineRenderFeature.h"
#include <Renderer/SceneRenderer.h>
#include <Renderer/Shader.h>

#include <Renderer/Types/Vertex.h>
#include <array>
#include <vector>

Moon::Rendering::DebugLineRenderFeature::DebugLineRenderFeature(CompositeRenderer& p_renderer, FeatureExecutionPolicy p_executionPolicy) :
	ARenderFeature(p_renderer, p_executionPolicy)
{
	
}

std::vector<DebugVertex> Moon::Rendering::DebugLineRenderFeature::GetLines() {
	return m_lines;
}

OpenGLDebugMesh Moon::Rendering::DebugLineRenderFeature::GetLinesMesh() {
	return m_debugLinesMesh;
}

void Moon::Rendering::DebugLineRenderFeature::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor) {
	m_debugLinesMesh.UpdateVertexData(m_lines);
	m_lines.clear();
}

void Moon::Rendering::DebugLineRenderFeature::AddLine(glm::vec3 start, glm::vec3 end, glm::vec3 color) {

	DebugVertex v0 = DebugVertex(start, color);
	DebugVertex v1 = DebugVertex(end, color);
	m_lines.push_back(v0);
	m_lines.push_back(v1);
}

void Moon::Rendering::DebugLineRenderFeature::DrawBox(const glm::vec3& p_position, const glm::quat& p_rotation, const glm::vec3& p_size, const glm::vec3& p_color, float p_lineWidth, bool p_depthTest) {
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, p_size.y, -p_size.z }, p_position + p_rotation  * glm::vec3{ -p_size.x, +p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ -p_size.x, +p_size.y, -p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * glm::vec3{ -p_size.x, +p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ +p_size.x, p_size.y, -p_size.z }, p_position + p_rotation  * glm::vec3{ +p_size.x, +p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, +p_size.y, -p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, +p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, -p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, +p_size.y, -p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, +p_size.y, -p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * glm::vec3{ +p_size.x, -p_size.y, +p_size.z }, p_color);
	AddLine(p_position + p_rotation * glm::vec3{ -p_size.x, +p_size.y, +p_size.z }, p_position + p_rotation*  glm::vec3{ +p_size.x, +p_size.y, +p_size.z }, p_color);
}

void Moon::Rendering::DebugLineRenderFeature::DrawSphere(const glm::vec3& p_position, const glm::quat& p_rotation, float p_radius, const glm::vec3& p_color, float p_lineWidth, bool p_depthTest) {
	if (!std::isinf(p_radius)) {
		for (float i = 0; i <= 360.0f; i += 10.0f) {
			AddLine(p_position + p_rotation * (glm::vec3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color);
			AddLine(p_position + p_rotation * (glm::vec3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (glm::vec3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);
			AddLine(p_position + p_rotation * (glm::vec3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);
		}
	}
}

void Moon::Rendering::DebugLineRenderFeature::DrawCapsule(const glm::vec3& p_position, const glm::quat& p_rotation, float p_radius, float p_height, const glm::vec3& p_color, float p_lineWidth, bool p_depthTest) {
	if (!std::isinf(p_radius)) {
		float halfHeight = p_height / 2;

		glm::vec3 hVec = { 0.0f, halfHeight, 0.0f };

		for (float i = 0; i < 360.0f; i += 10.0f) {
			AddLine(p_position + p_rotation * (hVec + glm::vec3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (hVec + glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);
			AddLine(p_position + p_rotation * (-hVec + glm::vec3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (-hVec + glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);

			if (i < 180.f) {
				AddLine(p_position + p_rotation * (hVec + glm::vec3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (hVec + glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color);
				AddLine(p_position + p_rotation * (hVec + glm::vec3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (hVec + glm::vec3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);
			}
			else {
				AddLine(p_position + p_rotation * (-hVec + glm::vec3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (-hVec + glm::vec3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color);
				AddLine(p_position + p_rotation * (-hVec + glm::vec3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (-hVec + glm::vec3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color);
			}
		}

		AddLine(p_position + p_rotation * (glm::vec3{ -p_radius, -halfHeight, 0.f }), p_position + p_rotation * (glm::vec3{ -p_radius, +halfHeight, 0.f }), p_color);
		AddLine(p_position + p_rotation * (glm::vec3{ p_radius, -halfHeight, 0.f }), p_position + p_rotation * (glm::vec3{ p_radius, +halfHeight, 0.f }), p_color);
		AddLine(p_position + p_rotation * (glm::vec3{ 0.f, -halfHeight, -p_radius }), p_position + p_rotation * (glm::vec3{ 0.f, +halfHeight, -p_radius }), p_color);
		AddLine(p_position + p_rotation * (glm::vec3{ 0.f, -halfHeight, p_radius }), p_position + p_rotation * (glm::vec3{ 0.f, +halfHeight, p_radius }), p_color);
	}
}

void Moon::Rendering::DebugLineRenderFeature::DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform) {
	glm::vec3 FrontTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
	glm::vec3 FrontTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
	glm::vec3 FrontBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
	glm::vec3 FrontBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
	glm::vec3 BackTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
	glm::vec3 BackTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
	glm::vec3 BackBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
	glm::vec3 BackBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
	AddLine(FrontTopLeft, FrontTopRight, color);
	AddLine(FrontBottomLeft, FrontBottomRight, color);
	AddLine(BackTopLeft, BackTopRight, color);
	AddLine(BackBottomLeft, BackBottomRight, color);
	AddLine(FrontTopLeft, FrontBottomLeft, color);
	AddLine(FrontTopRight, FrontBottomRight, color);
	AddLine(BackTopLeft, BackBottomLeft, color);
	AddLine(BackTopRight, BackBottomRight, color);
	AddLine(FrontTopLeft, BackTopLeft, color);
	AddLine(FrontTopRight, BackTopRight, color);
	AddLine(FrontBottomLeft, BackBottomLeft, color);
	AddLine(FrontBottomRight, BackBottomRight, color);
}
