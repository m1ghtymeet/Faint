#include "IconGenerator.h"
#include <Types/Renderer/Mesh.h>
#include <AssetManagment/MeshManager.h>
#include <Renderer/OpenGL/GL_BackEnd.h>

#include <Input/Input.h>
#include <Input/KeyCodes.h>

namespace MeshUtil {
    std::vector<Moon::Rendering::Vertex> GenerateSphereVertices(float radius, int segments) {
        std::vector<Moon::Rendering::Vertex> vertices;
        // Ensure segments are reasonable
        segments = std::max(segments, 4);
        // Angular step sizes
        float thetaStep = glm::two_pi<float>() / segments; // Longitude step
        float phiStep = glm::pi<float>() / segments;       // Latitude step
        for (int i = 0; i <= segments; ++i) { // Latitude loop
            float phi = i * phiStep; // Latitude angle
            for (int j = 0; j <= segments; ++j) { // Longitude loop
                float theta = j * thetaStep; // Longitude angle
                // Calculate position on the sphere
                glm::vec3 position = glm::vec3(
                    radius * sin(phi) * cos(theta),
                    radius * cos(phi),
                    radius * sin(phi) * sin(theta)
                );
                // Calculate normal (direction from sphere center)
                glm::vec3 normal = glm::normalize(position);
                // Calculate tangent (partial derivative with respect to theta)
                glm::vec3 tangent = glm::normalize(glm::vec3(
                    -radius * sin(phi) * sin(theta),
                    0.0f,
                    radius * sin(phi) * cos(theta)
                ));
                glm::vec2 uv(
                    theta / glm::two_pi<float>(),
                    phi / glm::pi<float>()
                );
                // Add vertex to the vector
                Moon::Rendering::Vertex& vertex = vertices.emplace_back();
                vertex.position = position;
                vertex.uv = uv;
                vertex.normal = normal;
                vertex.tangent = tangent;
            }
        }
        return vertices;
    }

    std::vector<uint32_t> GenerateSphereIndices(int segments) {
        std::vector<uint32_t> indices;
        for (int i = 0; i < segments; ++i) { // Latitude loop
            for (int j = 0; j < segments; ++j) { // Longitude loop
                int nextI = i + 1;
                int nextJ = (j + 1) % (segments + 1);
                // Indices of the quad
                uint32_t v0 = i * (segments + 1) + j;
                uint32_t v1 = nextI * (segments + 1) + j;
                uint32_t v2 = i * (segments + 1) + nextJ;
                uint32_t v3 = nextI * (segments + 1) + nextJ;
                // First triangle
                indices.push_back(v2);
                indices.push_back(v1);
                indices.push_back(v0);
                // Second triangle
                indices.push_back(v3);
                indices.push_back(v1);
                indices.push_back(v2);
            }
        }
        return indices;
    }
}

Moon::Mesh* defaultMaterial = nullptr;
uint32_t sphereMeshIndex;

IconGenerator::IconGenerator() {
	m_previewFrameBuffer = Moon::FrameBuffer("Icon", 256, 256);
	m_previewFrameBuffer.CreateAttachment("Color", GL_RGB16F);
    m_previewFrameBuffer.CreateDepthAttachment(GL_DEPTH_COMPONENT32F);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<Moon::Rendering::Vertex> vertices = MeshUtil::GenerateSphereVertices(1.0f, 15.0f);
    std::vector<uint32_t> indices = MeshUtil::GenerateSphereIndices(15.0f);
    sphereMeshIndex = Moon::AssetManagment::MeshManager::CreateMesh("Sphere", vertices, indices);

    m_shader = std::make_shared<Moon::Shader>(std::vector<std::string>{ "data/shaders/generator.ftshader" });
}

void IconGenerator::Generate(Moon::Rendering::Material& material) {

    if (Moon::Input::KeyPressed(Moon::Key::T)) {
        m_shader->Hotload();
    }

	m_previewFrameBuffer.Bind();
	glViewport(0, 0, 256, 256);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.12f, 0.12f, 0.12f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//constexpr float fov = glm::radians(38.0f);
	//glm::mat4 proj = glm::perspective(fov, 256.0f / 256.0f, 0.1f, 10.0f);
	//glm::vec3 camPos = { 0.0f, 0.0f, 2.2f };
	//glm::mat4 view = glm::lookAt(camPos, glm::vec3(0), glm::vec3(0, 1, 0));

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-18.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(1, 0, 0));

	m_shader->Bind();
	m_shader->SetMat4("u_modelMatrix", model);

	material.SetShader(m_shader);
	material.Bind();
    glBindVertexArray(Moon::Rendering::OpenGLBackEnd::GetVertexDataVAO());
    const auto& mesh = Moon::AssetManagment::MeshManager::GetMeshByIndex(sphereMeshIndex);
    glDrawElementsBaseVertex(GL_TRIANGLES,
        mesh->indexCount,
        GL_UNSIGNED_INT,
        (void*)(uintptr_t)(mesh->firstIndex * sizeof(uint32_t)),
        mesh->baseVertex);
    glBindVertexArray(0);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IconGenerator::Generate(Moon::Model& model) {
    m_previewFrameBuffer.Bind();
    glViewport(0, 0, 256, 256);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //constexpr float fov = glm::radians(38.0f);
    //glm::mat4 proj = glm::perspective(fov, 256.0f / 256.0f, 0.1f, 10.0f);
    //glm::vec3 camPos = { 0.0f, 0.0f, 2.2f };
    //glm::mat4 view = glm::lookAt(camPos, glm::vec3(0), glm::vec3(0, 1, 0));

    glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-18.0f), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-10.0f), glm::vec3(1, 0, 0));

    m_shader->Bind();
    m_shader->SetMat4("u_modelMatrix", modelMatrix);

    //for (auto& mesh : model.GetMeshes()) {
    //    mesh->Bind();
    //    glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
    //    mesh->Unbind();
    //}
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Moon::Rendering::Texture* IconGenerator::GenerateAndReturn(Moon::Rendering::Material& material) {
    Generate(material);

    Moon::Rendering::Texture* preview = new Moon::Rendering::Texture(Moon::Rendering::ETextureType::TEXTURE_2D);
    preview->Create({
        .width = 256,
        .height = 256,
        .internalFormat = GL_RGBA8,
        .useMipMaps = false
        });

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_previewFrameBuffer.GetHandle());
    preview->Bind();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 256, 256);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return preview;
}

Moon::Rendering::Texture* IconGenerator::GenerateAndReturn(Moon::Model& model) {
    Generate(model);

    Moon::Rendering::Texture* preview = new Moon::Rendering::Texture(Moon::Rendering::ETextureType::TEXTURE_2D);
    preview->Create({
        .width = 256,
        .height = 256,
        .internalFormat = GL_RGBA8,
        .useMipMaps = false
        });

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_previewFrameBuffer.GetHandle());
    preview->Bind();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 256, 256);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return preview;
}
