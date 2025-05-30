#include "hzpch.h"
#include "Renderer.h"
#include "Types/Material.h"
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
    }

    void EndScene() {
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

    void DrawQuad(Matrix4 transform)
    {
        //ZoneScoped;
        QuadMesh->Draw(nullptr, false);
    }

    void DrawCube(Matrix4 transform)
    {
        //ZoneScoped;
        CubeMesh->Draw(nullptr, false);
    }
}
