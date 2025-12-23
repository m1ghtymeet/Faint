#include "CompositeRenderer.h"
#include <Renderer/OpenGL/RenderPasses/GL_GeometryPass.h>
#include <Renderer/Util.h>
#include <Debug/Instrumentor.h>

#include <AssetManagment/MeshManager.h>
#include <Scene/Components/CMaterialRenderer.h>

#include <ranges>

#include <glad/glad.h>

Moon::Rendering::CompositeRenderer::CompositeRenderer() {
}

void Moon::Rendering::CompositeRenderer::BeginFrame(const Data::FrameDescriptor& p_frameData) {
	FT_PROFILE_FUNCTION()

	BaseRenderer::BeginFrame(p_frameData);

	for (const auto& feature : m_features | std::views::values) {
		if (feature->IsEnabled()) {
			feature->OnBeginFrame(p_frameData);
		}
	}

	for (const auto& pass : m_passes | std::views::values) {
		if (pass.second->m_enabled) {
			pass.second->OnBeginFrame(p_frameData);
		}
	}
}

void Moon::Rendering::CompositeRenderer::DrawFrame() {
	FT_CORE_ASSERT(!m_currentPass.has_value(), "Cannot draw frame while a pass is already begin drawn. ");

	auto frameStart = std::chrono::steady_clock::now();

	PrecomputeNextFrame();

	for (const auto& feature : m_features | std::views::values) {
		//if (feature->IsEnabledFor(typeid(m_currentPass.value()))) {
			feature->OnBeforeDraw();
		//}
	}

	for (const auto& pass : m_passes | std::views::values) {
		m_currentPass = *pass.second.get();

		if (pass.second->m_enabled) {
			pass.second->Draw();
		}

		m_currentPass.reset();

		//auto elapsed = std::chrono::steady_clock::now() - frameStart;
		//double elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();
		//if (elapsedMs > m_frameBudgetMs) {
		//	HZ_CORE_WARN("Frame budget exceeded: " + std::to_string(elapsedMs) + " ms");
		//	break; // Skip remaining passes if over budget
		//}
	}
}

void Moon::Rendering::CompositeRenderer::EndFrame() {

	for (const auto& pass : m_passes | std::views::values) {
		if (pass.second->m_enabled) {
			pass.second->OnEndFrame();
		}
	}

	for (const auto& feature : m_features | std::views::values) {
		if (feature->IsEnabled()) {
			feature->OnEndFrame();
		}
	}

	ClearDescriptors();
	BaseRenderer::EndFrame();
}

void Moon::Rendering::CompositeRenderer::DrawEntity(PipelineState state, const RenderItem& renderItem) {
	// ZoneScoped
	if (renderItem.meshIndex < 0 || !renderItem.material) return;
	
	Util::SetPipelineState(state);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	auto shader = renderItem.material->GetShader();
	shader->Bind();
	shader->SetMat4("u_modelMatrix", renderItem.transform.GetWorldMatrix());
	shader->SetInt("u_entityID", renderItem.id);

	renderItem.material->Bind();
	Mesh* mesh = AssetManagment::MeshManager::GetMeshByIndex(renderItem.meshIndex);
	//if (p_instances > 0)
	{
		glDrawElementsBaseVertex(GL_TRIANGLES,
			mesh->indexCount,
			GL_UNSIGNED_INT,
			(void*)(uintptr_t)(mesh->firstIndex * sizeof(uint32_t)),
			mesh->baseVertex);
	}
	renderItem.material->Unbind();
}

void Moon::Rendering::CompositeRenderer::SetFrameBudget(double ms) {
	m_frameBudgetMs = ms;
}

void Moon::Rendering::CompositeRenderer::PrecomputeNextFrame() {
	//if (m_asyncPrecompute.valid() &&
	//	m_asyncPrecompute.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
	//	m_asyncPrecompute.get();
	//}
}

std::shared_ptr<Moon::Shader> Moon::Rendering::CompositeRenderer::CreateShader(const std::string& name, const std::vector<std::string>& paths) {
	auto it = m_shaders.find(name);
	if (it != m_shaders.end())
		return it->second;
	auto shader = std::make_shared<Moon::Shader>(paths);
	m_shaders[name] = shader;
	return shader;
}

std::shared_ptr<Moon::Shader> Moon::Rendering::CompositeRenderer::GetShader(const std::string& name) const {
	auto it = m_shaders.find(name);
	if (it != m_shaders.end())
		return it->second;
	return nullptr;
}

void Moon::Rendering::CompositeRenderer::HotReloadShaders() {
	for (auto& [name, shader] : m_shaders)
		shader->Hotload();
}

std::shared_ptr<Moon::FrameBuffer> Moon::Rendering::CompositeRenderer::CreateFrameBuffer(const std::string& name, uint32_t width, uint32_t height) {
	auto it = m_framebuffers.find(name);
	if (it != m_framebuffers.end()) {
		return it->second;
	}
	auto fb = std::make_shared<FrameBuffer>(name.c_str(), width, height);
	m_framebuffers[name] = fb;
	return fb;
}

std::shared_ptr<Moon::FrameBuffer> Moon::Rendering::CompositeRenderer::GetFrameBuffer(const std::string& name) const {
	auto it = m_framebuffers.find(name);
	if (it != m_framebuffers.end()) {
		return it->second;
	}
	return nullptr;
}
