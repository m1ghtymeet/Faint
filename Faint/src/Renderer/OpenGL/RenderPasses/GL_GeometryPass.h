#pragma once

#include <Renderer/Core/ARenderPass.h>
#include <Types/Types.h>
#include <Renderer/Types/Material.h>
#include <Types/Renderer/Mesh.h>
#include <Scene/Components/CModelRenderer.h>
#include <Math/Transform.h>
#include <memory>
#include <vector>
#include <atomic>

namespace Moon::Rendering {

    struct RenderQueue {
        std::vector<RenderItem> opaque;
        std::vector<RenderItem> transparent;

        void Clear() {
            opaque.clear();
            transparent.clear();
        }

        void Reserve(size_t opaqueSize, size_t transparentSize) {
            opaque.reserve(opaqueSize);
            transparent.reserve(transparentSize);
        }
    };

    class GeometryRenderPass : public ARenderPass {
    public:
        explicit GeometryRenderPass(CompositeRenderer& p_renderer);
        ~GeometryRenderPass() override;

        void Draw() override;
        uint32_t GetNormalTextureID();
        uint32_t GetWorldSpacePositionTextureID();
        uint32_t GetScreenSpacePositionTextureID();

        RenderQueue m_renderQueue;

        // Performance monitoring
        struct Stats {
            std::atomic<uint64_t> drawCalls{ 0 };
            std::atomic<uint64_t> trianglesRendered{ 0 };
            std::atomic<uint64_t> stateChanges{ 0 };
            std::atomic<float> cullTime{ 0.0f };
            std::atomic<float> sortTime{ 0.0f };
            std::atomic<float> drawTime{ 0.0f };
        };

        const Stats& GetStats() const { return m_stats; }
        void ResetStats() {
            m_stats.drawCalls = 0;
            m_stats.trianglesRendered = 0;
            m_stats.stateChanges = 0;
            m_stats.cullTime = 0.0f;
            m_stats.sortTime = 0.0f;
            m_stats.drawTime = 0.0f;
        }

    private:
        // Core rendering
        std::shared_ptr<Shader> m_defaultShader;

        // VAO caching
        std::unordered_map<Mesh*, GLuint> m_globalVaoCache;
        std::mutex m_vaoCacheMutex;

        // State management
        struct RenderState {
            const Material* material = nullptr;
            const Shader* shader = nullptr;
            bool depthTest = true;
            bool depthWrite = true;
            bool blending = false;
            bool culling = true;
            uint32_t vao = 0;
        };
        RenderState m_currentState;

        // Statistics
        Stats m_stats;

        // Private methods
        void PrecomputeFrame();
        void ProcessModelRenderer(const MeshRendererComponent* modelRenderer, const glm::vec3& cameraPos);
        void DrawRenderQueue(const RenderQueue& queue);
        void DrawRenderItem(const RenderItem& item);
        void ApplyMaterialState(const Material& material);
        void ResetRenderState();
        GLuint GetOrCreateVAO(Mesh* mesh);
        bool IsInFrustum(const Transform& transform, const Mesh* mesh) const;
    };

}