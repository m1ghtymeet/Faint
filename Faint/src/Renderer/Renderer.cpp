#include "hzpch.h"
#include "Renderer.h"
#include "Types/Material.h"
#include "ShadersManager.h"
#include "Util/Util.h"

#include "RenderCommand.h"
#include <glad/glad.h>

namespace Faint::Renderer {

    RendererAPI m_api = RendererAPI::OPENGL;

    RenderList m_RenderList;

    std::vector<Vertex> QuadVertices = {
        // Position               Normal               UV            Tangent
        {{-1.0f, -1.0f, 0.0f},    {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-left
        {{ 1.0f, -1.0f, 0.0f},    {0.0f, 0.0f, 1.0f},  {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom-right
        {{ 1.0f,  1.0f, 0.0f},    {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}, // Top-right
        {{-1.0f,  1.0f, 0.0f},    {0.0f, 0.0f, 1.0f},  {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}  // Top-left
    };

    RendererAPI GetAPI() {
        return RendererAPI::OPENGL;
    }

    void Init() {
        
        m_DefaultFont = AssetManager::LoadFont("data/editor/fonts/FBBB.ttf");

        Ref<Material> defaultMaterial = CreateRef<Material>(Vec3(1, 1, 1));
        defaultMaterial->SetName("white");

        CubeMesh = CreateRef<Mesh>();
        CubeMesh->AddSurface(Util::GenerateCubeVertices(), Util::GenerateCubeIndices());
        CubeMesh->SetMaterial(defaultMaterial);

        QuadMesh = CreateRef<Mesh>();
        QuadMesh->AddSurface(QuadVertices, { 0, 1, 2, 2, 3, 0 });
        QuadMesh->SetMaterial(defaultMaterial);

        SphereMesh = CreateRef<Mesh>();
        SphereMesh->AddSurface(Util::GenerateSphereVertices(1, 10), Util::GenerateSphereIndices(10));
        SphereMesh->SetMaterial(CreateRef<Material>());
    }

    void SetViewport(float x, float y, float width, float height) {
        glViewport((GLint)x, (GLint)y, (GLint)width, (GLint)height);
    }

    void BeginScene() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.10588, 0.10588, 0.10588, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    void EndScene() {

        Shader deferredShader = ShaderManager::GetShader("deferred");
        deferredShader.Bind();
        deferredShader.SetInt("LightCount", 0);

        for (int i = 0; i < m_Lights.size(); i++)
        {
            const std::string uniformAccessor = "Lights[" + std::to_string(i) + "].";
            deferredShader.SetVec3(uniformAccessor + "Position", Vec3(0));
            deferredShader.SetVec3(uniformAccessor + "Color", Vec3(0));
            deferredShader.SetInt(uniformAccessor + "Type", -1);
            deferredShader.SetInt(uniformAccessor + "CastShadow", 0);
            deferredShader.SetInt(uniformAccessor + "ShadowMapID", -1);
        }

        m_Lights.clear();
    }

    void SubmitMesh(Ref<Mesh> mesh, const Matrix4& transform, const int32_t entityID) {
        m_RenderList.AddToRenderList(mesh, transform, entityID);
    }

    void Flush(Shader* shader, bool depthOnly) {
        m_RenderList.Flush(shader, depthOnly);
    }

    void OnWindowResize(uint32_t width, uint32_t height) {
        SetViewport(0, 0, (float)width, (float)height);
    }

    void RegisterLight(TransformComponent transform, LightComponent light) {
        Shader deferredShader = ShaderManager::GetShader("deferred");
        deferredShader.Bind();

        Vec3 direction = light.Direction;
        Vec3 pos = transform.GetGlobalPosition();
        Quat lightRotation = transform.GetGlobalRotation();

        
        if (light.Type == Directional) {

            deferredShader.SetVec3("a_DirectionalLight.Direction", direction);
            deferredShader.SetVec3("a_DirectionalLight.Color", light.Color);
        }
        else {
            if (m_Lights.size() == MAX_LIGHTS)
                return;

            m_Lights.push_back({ transform, light });
            size_t idx = m_Lights.size();

            const std::string uniformAccessor = "Lights[" + std::to_string(idx - 1) + "].";
            deferredShader.SetVec3(uniformAccessor + "Position", pos);
            deferredShader.SetVec3(uniformAccessor + "Color", Vec3(light.Color /** light.Strength*/));
            deferredShader.SetFloat(uniformAccessor + "Radius", light.Radius);
            deferredShader.SetFloat(uniformAccessor + "Strength", light.Strength);
            deferredShader.SetInt(uniformAccessor + "Type", static_cast<int>(light.Type));
            deferredShader.SetInt(uniformAccessor + "CastShadow", static_cast<int>(light.CastShadows));

            deferredShader.SetMat4("lightProjectionView", light.mViewProjections[0]);

            const int MaxSpotShadowMap = 8;
            int spotShadowMapCount = 0;
            if (light.Type == Point) {

                if (light.CastShadows) {

                    int shadowMapTextureSlot = 21;
                    deferredShader.SetInt(uniformAccessor + "ShadowMapID", 1);
                    //deferredShader->SetInt(uniformAccessor + "Transform", light);
                    
                    light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(shadowMapTextureSlot);
                    deferredShader.SetInt("ShadowMaps[" + std::to_string(0) + "]", shadowMapTextureSlot);
                    
                }
            }
            else if (light.Type == Spot) {

                direction = transform.GetGlobalRotation() * Vec3(0, 0, -1);
                deferredShader.SetVec3(uniformAccessor + "Direction", direction);
                deferredShader.SetFloat(uniformAccessor + "OuterAngle", glm::cos(Rad(light.Outercutoff)));
                deferredShader.SetFloat(uniformAccessor + "InnerAngle", glm::cos(Rad(light.Cutoff)));

                if (light.CastShadows && spotShadowMapCount < MaxSpotShadowMap)
                {
                    int shadowMapTextureSlot = 21 + spotShadowMapCount;
                    deferredShader.SetInt(uniformAccessor + "ShadowMapID", spotShadowMapCount);
                    deferredShader.SetMat4(uniformAccessor + "Transform", light.GetProjection() * glm::inverse(transform.GetGlobalMatrix()));

                    light.m_Framebuffers[0]->GetTexture(GL_DEPTH_ATTACHMENT)->Bind(shadowMapTextureSlot);
                    deferredShader.SetInt("SpotShadowMaps[" + std::to_string(spotShadowMapCount) + "]", shadowMapTextureSlot);
                    spotShadowMapCount++;
                }
            }
            else {
                deferredShader.SetVec3(uniformAccessor + "Direction", Vec3(0, 0, 0));
                deferredShader.SetFloat(uniformAccessor + "OuterAngle", glm::cos(Rad(light.Outercutoff)));
                deferredShader.SetFloat(uniformAccessor + "InnerAngle", glm::cos(Rad(light.Cutoff)));
                deferredShader.SetInt(uniformAccessor + "ShadowMapID", -1);
            }

            deferredShader.SetInt("LightCount", static_cast<int>(idx));
        }
    }

    void DrawLine(Vec3 begin, Vec3 end, Vec3 color, bool obeyDepth) {
        Vertex v0 = Vertex(begin, color, glm::ivec2(0, 0));
        Vertex v1 = Vertex(end, color, glm::ivec2(0, 0));
        if (obeyDepth) {

        }
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color) {
        glm::vec3 FrontTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 BackTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        DrawLine(FrontTopLeft, FrontTopRight, color);
        DrawLine(FrontBottomLeft, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackTopRight, color);
        DrawLine(BackBottomLeft, BackBottomRight, color);
        DrawLine(FrontTopLeft, FrontBottomLeft, color);
        DrawLine(FrontTopRight, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackBottomLeft, color);
        DrawLine(BackTopRight, BackBottomRight, color);
        DrawLine(FrontTopLeft, BackTopLeft, color);
        DrawLine(FrontTopRight, BackTopRight, color);
        DrawLine(FrontBottomLeft, BackBottomLeft, color);
        DrawLine(FrontBottomRight, BackBottomRight, color);
    }

    void DrawAABB(const AABB& aabb, const glm::vec3& color, const glm::mat4& worldTransform) {
        glm::vec4 FrontTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec4 FrontTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec4 FrontBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec4 FrontBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z, 1.0f);
        glm::vec4 BackTopLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec4 BackTopRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec4 BackBottomLeft = worldTransform * glm::vec4(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
        glm::vec4 BackBottomRight = worldTransform * glm::vec4(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z, 1.0f);
        DrawLine(FrontTopLeft, FrontTopRight, color);
        DrawLine(FrontBottomLeft, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackTopRight, color);
        DrawLine(BackBottomLeft, BackBottomRight, color);
        DrawLine(FrontTopLeft, FrontBottomLeft, color);
        DrawLine(FrontTopRight, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackBottomLeft, color);
        DrawLine(BackTopRight, BackBottomRight, color);
        DrawLine(FrontTopLeft, BackTopLeft, color);
        DrawLine(FrontTopRight, BackTopRight, color);
        DrawLine(FrontBottomLeft, BackBottomLeft, color);
        DrawLine(FrontBottomRight, BackBottomRight, color);
    }

    void DrawQuad(Matrix4 transform)
    {
        //ZoneScoped;
        QuadMesh->Bind();
        QuadMesh->Draw(nullptr, false);
    }

    void DrawCube(Matrix4 transform)
    {
        //ZoneScoped;
        CubeMesh->Bind();
        CubeMesh->Draw(nullptr, false);
    }
}
