#include "GL_GeometryPass.h"
#include <algorithm>

#include <AssetManagment/MeshManager.h>
#include <Renderer/SceneRenderer.h>
#include <Renderer/Features/EntityRenderFeature.h>
#include <Scene/Components/CMaterialRenderer.h>
#include <Math/Frustum.h>

#include <Renderer/OpenGL/GL_BackEnd.h>
#include <Renderer/OpenGL/RenderPasses/GL_ShadowMapRenderPass.h>

namespace Moon::Rendering {

    GeometryRenderPass::GeometryRenderPass(CompositeRenderer& p_renderer)
        : ARenderPass(p_renderer)
    {
        //HZ_CORE_INFO("Initializing GeometryRenderPass...");

        // Pre-warm shader
        m_defaultShader = m_renderer.GetShader("gBuffer");
        m_defaultShader->Bind();
        m_defaultShader->Unbind();

        // Single render queue
        m_renderQueue.Reserve(2048, 512);
    }

    GeometryRenderPass::~GeometryRenderPass() {
        // Clean up VAO cache
        for (auto& [mesh, vao] : m_globalVaoCache) {
            glDeleteVertexArrays(1, &vao);
        }
    }

    void GeometryRenderPass::Draw() {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Bind framebuffer and clear
        FrameBuffer* framebuffer = m_renderer.GetFrameBuffer("GBuffer").get();
        framebuffer->Bind();

        //glClearColor(0, 0, 0, 1);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Precompute current frame (simple & synchronous)
        PrecomputeFrame();

        // Setup rendering
        auto entityFeature = m_renderer.GetFeature<EntityRenderFeature>();
        if (auto camera = m_renderer.GetFrameDescriptor().camera) {
            entityFeature.SetCamera(*camera);
        }

        framebuffer->DrawBuffers(
            { "Albedo", "Normal", "RMA", "WorldSpacePosition", "ScreenSpacePosition", "MousePick" }
        );

        // Bind shadow map
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP,
            m_renderer.GetPass<ShadowMapRenderPass>("ShadowMap").GetID());

        // Reset state
        ResetRenderState();

        // Draw queues
        auto drawStart = std::chrono::high_resolution_clock::now();
        DrawRenderQueue(m_renderQueue);
        auto drawEnd = std::chrono::high_resolution_clock::now();

        m_stats.drawTime = std::chrono::duration<float, std::milli>(drawEnd - drawStart).count();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Calculate total frame time
        auto frameEnd = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();

        if (frameTime > 16.67f) {
            HZ_CORE_WARN("Frame time: {:.2f}ms (target: 16.67ms)", frameTime);
        }
    }

    void GeometryRenderPass::PrecomputeFrame() {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Clear queue
        m_renderQueue.Clear();

        const auto& scene = m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>().scene;
        if (!scene) return;

        const auto& modelRenderers = scene->GetFastAccessComponents().modelRenderers;
        if (modelRenderers.empty()) return;

        const auto& camera = m_renderer.GetFrameDescriptor().camera;
        const glm::vec3 cameraPos = camera ? camera->GetPosition() : glm::vec3(0);

        // Process all model renderers
        for (const auto* modelRenderer : modelRenderers) {
            ProcessModelRenderer(modelRenderer, cameraPos);
        }

        // Sort transparent objects back-to-front
        auto sortStart = std::chrono::high_resolution_clock::now();

        if (!m_renderQueue.transparent.empty() && camera) {
            std::sort(m_renderQueue.transparent.begin(), m_renderQueue.transparent.end(),
                [](const RenderItem& a, const RenderItem& b) {
                    return a.distanceToCamera > b.distanceToCamera;
                });
        }

        // Sort opaque by material/shader to minimize state changes
        std::sort(m_renderQueue.opaque.begin(), m_renderQueue.opaque.end(),
            [](const RenderItem& a, const RenderItem& b) {
                if (a.material->GetShader() != b.material->GetShader())
                    return a.material->GetShader() < b.material->GetShader();
                if (a.material != b.material)
                    return a.material < b.material;
                return a.meshIndex < b.meshIndex;
            });

        auto sortEnd = std::chrono::high_resolution_clock::now();
        m_stats.sortTime = std::chrono::duration<float, std::milli>(sortEnd - sortStart).count();
        m_stats.cullTime = std::chrono::duration<float, std::milli>(sortStart - startTime).count();
    }

    void GeometryRenderPass::ProcessModelRenderer(const MeshRendererComponent* modelRenderer, const glm::vec3& cameraPos) {
        if (!modelRenderer || !modelRenderer->owner.IsActive() || !modelRenderer->GetModel())
            return;

        const auto materialRenderer = modelRenderer->owner.GetComponent<MaterialRendererComp>();
        if (!materialRenderer) return;

        const auto& materials = materialRenderer->GetMaterials();
        const auto& transform = modelRenderer->owner.transform->GetTransform();
        const uint32_t entityID = modelRenderer->owner.GetID();

        //if (!m_frustum.IsAABBVisible(modelRenderer->GetModel()->GetWorldAAB(transform.GetWorldMatrix())))
        //    return;

        for (const auto& mesh : modelRenderer->GetModel()->GetMeshIndices()) {
            uint32_t materialIndex = Moon::AssetManagment::MeshManager::GetMeshByIndex(mesh)->materialIndex;
            if (materialIndex >= materials.size() || !materials[materialIndex])
                continue;

            auto material = materials[materialIndex];

            // Calculate distance for transparent sorting
            float distance = 0.0f;
            if (material->IsBlendable()) {
                glm::vec3 worldPos = glm::vec3(transform.GetWorldMatrix()[3]);
                distance = glm::distance2(cameraPos, worldPos);
            }

            RenderItem item{
                .meshIndex = mesh,
                .material = material,
                .transform = transform,
                .id = entityID,
                .distanceToCamera = distance
            };

            if (material->IsBlendable()) {
                m_renderQueue.transparent.push_back(item);
            }
            else {
                m_renderQueue.opaque.push_back(item);
            }
        }
    }

    void GeometryRenderPass::DrawRenderQueue(const RenderQueue& queue) {
        glBindVertexArray(Rendering::OpenGLBackEnd::GetVertexDataVAO());
        // Draw opaque first
        for (const auto& item : queue.opaque) {
            DrawRenderItem(item);
        }

        // Draw transparent
        for (const auto& item : queue.transparent) {
            DrawRenderItem(item);
        }
        glBindVertexArray(0);
    }

    void GeometryRenderPass::DrawRenderItem(const RenderItem& item) {
        if (item.meshIndex < 0 || !item.material) return;

        // Apply material state changes
        ApplyMaterialState(*item.material);

        // Set shader
        auto shader = item.material->GetShader();
        if (!shader) {
            shader = m_defaultShader;
            item.material->SetShader(shader);
        }

        shader->Bind();

        // Set uniforms
        shader->SetMat4("u_modelMatrix", item.transform.GetWorldMatrix());
        shader->SetInt("u_entityID", item.id);

        // Bind material
        item.material->m_systemTextures["Lighting"] = m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("FinalLighting");
        item.material->Bind();
        // Draw mesh
        const auto& mesh = AssetManagment::MeshManager::GetMeshByIndex(item.meshIndex);
        glDrawElementsBaseVertex(GL_TRIANGLES,
            mesh->indexCount,
            GL_UNSIGNED_INT,
            (void*)(uintptr_t)(mesh->firstIndex * sizeof(uint32_t)),
            mesh->baseVertex);

        m_stats.drawCalls++;
        m_stats.trianglesRendered += mesh->indexCount / 3;

        item.material->Unbind();
    }

    void GeometryRenderPass::ApplyMaterialState(const Material& material) {
        if (m_currentState.material == &material) return;

        // Depth test
        bool needsDepthTest = material.HasDepthTest();
        if (m_currentState.depthTest != needsDepthTest) {
            needsDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            m_currentState.depthTest = needsDepthTest;
            m_stats.stateChanges++;
        }

        // Depth write
        bool needsDepthWrite = material.HasDepthWriting();
        if (m_currentState.depthWrite != needsDepthWrite) {
            glDepthMask(needsDepthWrite ? GL_TRUE : GL_FALSE);
            m_currentState.depthWrite = needsDepthWrite;
            m_stats.stateChanges++;
        }

        // Blending
        bool needsBlending = material.IsBlendable();
        if (m_currentState.blending != needsBlending) {
            if (needsBlending) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else {
                glDisable(GL_BLEND);
            }
            m_currentState.blending = needsBlending;
            m_stats.stateChanges++;
        }

        // Culling
        bool needsCulling = material.HasBackfaceCulling();
        if (m_currentState.culling != needsCulling) {
            if (needsCulling) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
            else {
                glDisable(GL_CULL_FACE);
            }
            m_currentState.culling = needsCulling;
            m_stats.stateChanges++;
        }

        m_currentState.material = &material;
    }

    void GeometryRenderPass::ResetRenderState() {
        m_currentState = RenderState{};
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
}

bool Moon::Rendering::GeometryRenderPass::IsInFrustum(const Transform& transform, const Mesh* mesh) const {    
    return true;
}

uint32_t Moon::Rendering::GeometryRenderPass::GetNormalTextureID() {
    return m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("Normal");
}

uint32_t Moon::Rendering::GeometryRenderPass::GetWorldSpacePositionTextureID() {
    return m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("WorldSpacePosition");
}

uint32_t Moon::Rendering::GeometryRenderPass::GetScreenSpacePositionTextureID() {
    return m_renderer.GetFrameBuffer("GBuffer")->GetColorAttachmentHandleByName("ScreenSpacePosition");
}
