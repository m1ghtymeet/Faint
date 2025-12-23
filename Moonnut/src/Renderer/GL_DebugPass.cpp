#include "GL_DebugPass.h"
#include <Renderer/SceneRenderer.h>

#include <Physics/PhysicsEngine.h>
#include <Core/GlobalLocator.h>
#include <Scene/Entity.h>
#include <Scene/Components/CBoxCollider.h>
#include <Scene/Components/CCharacterController.h>
#include <Scene/Components/CPointLight.h>
#include <Scene/Components/CDirectionalLight.h>
#include <Renderer/Features/DebugLineRenderFeature.h>
#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <AssetManagment/MeshManager.h>
#include <AssetManagment/Loader/ModelLoader.h>

#include "../Renderer/Types.h"

namespace {
	Moon::Shader* shader = nullptr;
	Moon::Shader* gridShader;
	Moon::Shader* axisShader = nullptr;
	GLuint emptyVAO;

	Moon::Model* g_cameraModel;

	glm::mat4 CalculateCameraModelMatrix(Moon::Entity& entity) {
		const glm::vec3 position = entity.transform->GetGlobalPosition();
		const glm::quat rotation = entity.transform->GetGlobalRotation();

		glm::quat modelFix = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1));
		modelFix = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0)) * modelFix;
		const glm::quat finalRotation = rotation * modelFix;
		return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(finalRotation) * glm::scale(glm::mat4(1), glm::vec3(0.25f));
	}
}

Moon::Rendering::DebugRenderPass::DebugRenderPass(CompositeRenderer& p_renderer) :
	ARenderPass(p_renderer) {

	shader = m_renderer.CreateShader("debug_vertex", { "data/shaders/debug_vertex.ftshader" }).get();
	gridShader = m_renderer.CreateShader("grid", { "data/shaders/grid.ftshader" }).get();
	axisShader = new Shader({ "data/shaders/axis.vert", "data/shaders/axis.frag" });

	glGenVertexArrays(1, &emptyVAO);

	g_cameraModel = Moon::Loaders::ModelLoader::Create("data/editor/models/Camera.fbx", false);
}

void Moon::Rendering::DebugRenderPass::Draw() {
	/* ================== Physics ================== */
	using namespace physx;
	
	PxScene* pxScene = FTSERVICE(Physics::PhysicsEngine).PxScene();
	const PxRenderBuffer& renderBuffer = pxScene->getRenderBuffer();
	int vertexCount = renderBuffer.getNbLines() * 2;
	
	auto& debugFeature = m_renderer.GetFeature<DebugLineRenderFeature>();

	if (selected) {
		DrawEntityDebugElements(*selected);
	}

	m_renderer.GetFrameBuffer("GBuffer")->Bind();
	m_renderer.GetFrameBuffer("GBuffer")->DrawBuffers({"Normal", "FinalLighting", "MousePick"});
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	m_renderer.GetShader("default")->Bind();
	m_renderer.GetShader("default")->SetVec3("u_AlbedoColor", { 0, 0.447f, 1 });
	for (auto& cameraComp : m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene->GetFastAccessComponents().cameras) {
		auto& transform = cameraComp->owner.transform;
		glm::mat4 cameraModelMatrix = CalculateCameraModelMatrix(cameraComp->owner);
		m_renderer.GetShader("default")->SetMat4("u_modelMatrix", cameraModelMatrix);
		m_renderer.GetShader("default")->SetInt("u_entityID", cameraComp->owner.GetID());
		glBindVertexArray(Rendering::OpenGLBackEnd::GetVertexDataVAO());
		for (auto& meshIndex : g_cameraModel->GetMeshIndices()) {
			auto* mesh = AssetManagment::MeshManager::GetMeshByIndex(meshIndex);
			glDrawElementsBaseVertex(GL_TRIANGLES,
				mesh->indexCount,
				GL_UNSIGNED_INT,
				(void*)(uintptr_t)(mesh->firstIndex * sizeof(uint32_t)),
				mesh->baseVertex);
		}
		glBindVertexArray(0);
	}
	m_renderer.GetShader("default")->Unbind();

	m_renderer.GetFrameBuffer("GBuffer")->DrawBuffer("FinalLighting");
	glDisable(GL_CULL_FACE);
	shader->Bind();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	shader->SetFloat("u_colorEffect", 1.0f);
	if (debugFeature.GetLinesMesh().GetVertexCount() > 0) {
		glBindVertexArray(debugFeature.GetLinesMesh().GetVAO());
		glDrawArrays(GL_LINES, 0, debugFeature.GetLinesMesh().GetVertexCount());
	}

	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_ONE, GL_ONE);
	shader->SetFloat("u_colorEffect", 2.0f);
	if (debugFeature.GetLinesMesh().GetVertexCount() > 0) {
		glBindVertexArray(debugFeature.GetLinesMesh().GetVAO());
		glDrawArrays(GL_LINES, 0, debugFeature.GetLinesMesh().GetVertexCount());
	}
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	DrawGrid();

	/* ==== Axis ==== */
	axisShader->Bind();
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glDisable(GL_DEPTH_TEST);
}

void Moon::Rendering::DebugRenderPass::DrawGrid() {
	gridShader->Bind();
	gridShader->SetVec3("u_cameraPos", m_renderer.GetFrameDescriptor().camera->GetPosition());
	gridShader->SetVec2("gScreenSize", { m_renderer.GetFrameDescriptor().renderWidth, m_renderer.GetFrameDescriptor().renderHeight });

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderer.GetFrameBuffer("GBuffer")->GetDepthAttachmentHandle());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

void Moon::Rendering::DebugRenderPass::DrawEntityDebugElements(Entity& entity) {

	if (!entity.IsActive())
		return;

	auto& debugFeature = m_renderer.GetFeature<DebugLineRenderFeature>();
	glm::vec3 color = { 0, 1, 0 };

	if (auto* meshComp = entity.GetComponent<MeshRendererComponent>()) {
		if (meshComp->GetModel()) {
			auto& transform = entity.transform;
			glm::mat4 world = transform->GetGlobalMatrix();
			for (auto& meshIndex : meshComp->GetModel()->GetMeshIndices()) {
				Mesh* mesh = AssetManagment::MeshManager::GetMeshByIndex(meshIndex);
				debugFeature.DrawAABB(AABB{ mesh->aabbMin, mesh->aabbMax }, {1, 1, 0}, world);
			}
		}
	}
	if (auto* box = entity.GetComponent<BoxColliderComponent>()) {
		auto& transform = entity.transform;
		glm::mat4 world = transform->GetGlobalMatrix();
		glm::vec3 position = glm::vec3(world[3]);
		glm::quat rotation = glm::quat_cast(world);
		glm::vec3 size = box->GetSize();
		glm::vec3 offset = box->shapeOffset;
		position += rotation * offset;
		debugFeature.DrawBox(position, rotation, size, color);
		glm::vec3 localCorners[8] = {
			{-size.x, -size.y, -size.z},
			{ size.x, -size.y, -size.z},
			{ size.x,  size.y, -size.z},
			{-size.x,  size.y, -size.z},
			{-size.x, -size.y,  size.z},
			{ size.x, -size.y,  size.z},
			{ size.x,  size.y,  size.z},
			{-size.x,  size.y,  size.z}
		};

		//auto& colliderGizmo = m_renderer.GetDescriptor<ColliderGizmoState>();
		//for (int i = 0; i < 8; i++) {
		//	glm::vec3 worldPos = position + rotation * localCorners[i];
		//	colliderGizmo.handlePositions[i] = worldPos;
		//
		//	glm::vec3 color = 
		//		(i == colliderGizmo.selectedHandle) ? glm::vec3(1, 1, 0) :
		//		(i == colliderGizmo.hoveredHandle) ? glm::vec3(0, 0.5, 1) :
		//		glm::vec3(0.7, 0.7, 0.7);
		//
		//	debugFeature.DrawSphere(worldPos, glm::quat(), 0.05f, color);
		//}
	}
	if (auto* controller = entity.GetComponent<CharacterControllerComponent>()) {
		auto& cc = controller->GetCharacterController();
		const auto& settings = cc->GetSettings();
		glm::vec3 pos = entity.transform->GetGlobalPosition();
		float fullHeight = settings.height + 2.0f * settings.radius;
		glm::vec3 center = pos + glm::vec3(0, fullHeight * 0.5f - settings.radius, 0);
		debugFeature.DrawCapsule(center, glm::quat(), settings.radius, fullHeight * 0.5f, color);
	}
	if (auto* light = entity.GetComponent<PointLightComponent>()) {
		auto& transform = entity.transform;
		glm::mat4 world = transform->GetGlobalMatrix();
		glm::vec3 position = glm::vec3(world[3]);
		glm::quat rotation = glm::quat_cast(world);
		debugFeature.DrawSphere(position, rotation, light->GetData().radius, {1, 1, 0});
	}
	if (auto* light = entity.GetComponent<DirectionalLightComp>()) {
		auto& transform = entity.transform;
		glm::vec3 pos = transform->GetGlobalPosition();
		glm::quat rot = transform->GetGlobalRotation();

		glm::vec3 forward = glm::normalize(rot * glm::vec3(0, 0, -1));
		glm::vec3 dirToScene = forward;

		const float sunSize = 1.5f;
		debugFeature.DrawSphere(pos, rot, sunSize, glm::vec3(1.0f, 0.9f, 0.6f));

		for (int i = 0; i < 12; ++i) {
			float angle = i * 30.0f * 3.14159f / 180.0f;
			glm::vec3 offset = glm::vec3(cos(angle), sin(angle), 0) * sunSize * 1.8f;
			debugFeature.AddLine(pos, pos + rot * offset, glm::vec3(1.0f, 0.8f, 0.3f));
		}

		const float arrowLength = 12.0f;
		glm::vec3 end = pos + dirToScene * arrowLength;

		debugFeature.AddLine(pos, end, glm::vec3(1.0f, 0.95f, 0.7f));

		glm::vec3 arrowHead = end - dirToScene * 2.0f;
		glm::vec3 right = glm::normalize(glm::cross(dirToScene, glm::vec3(0, 1, 0))) * 1.2f;
		glm::vec3 up = glm::normalize(glm::cross(dirToScene, right)) * 1.2f;

		debugFeature.AddLine(end, arrowHead + right, glm::vec3(1.0f, 0.9f, 0.5f));
		debugFeature.AddLine(end, arrowHead - right, glm::vec3(1.0f, 0.9f, 0.5f));
		debugFeature.AddLine(end, arrowHead + up, glm::vec3(1.0f, 0.9f, 0.5f));
		debugFeature.AddLine(end, arrowHead - up, glm::vec3(1.0f, 0.9f, 0.5f));

		if (light->GetData().castShadows) {
			float size = light->GetData().shadowAreaSize * 0.5f;
			glm::vec3 center = pos + dirToScene * 20.0f;
			glm::quat frustumRot = glm::quatLookAt(dirToScene, glm::vec3(0, 1, 0));
			debugFeature.DrawBox(center, frustumRot, glm::vec3(size, size, 30.0f), glm::vec3(0.8f, 0.4f, 0.9f), 2.0f, false);
		}
	}

	for (auto& child : entity.GetChildren()) {
		DrawEntityDebugElements(*child);
	}
}
