#include "GL_BackEnd.h"

#include <glad/glad.h>

namespace Moon::Rendering::OpenGLBackEnd {
    GLuint g_vertexDataVAO = 0;
    GLuint g_vertexDataVBO = 0;
    GLuint g_vertexDataEBO = 0;
    GLuint g_weightedVertexDataVAO = 0;
    GLuint g_weightedVertexDataVBO = 0;
    GLuint g_weightedVertexDataEBO = 0;
}

void Moon::Rendering::OpenGLBackEnd::InitGlobalBuffers() {
    if (g_vertexDataVAO != 0) {
        //glDeleteVertexArrays(1, &g_vertexDataVAO);
        //glDeleteBuffers(1, &g_vertexDataVBO);
        //glDeleteBuffers(1, &g_vertexDataEBO);
    }

    // Vertex
    glGenVertexArrays(1, &g_vertexDataVAO);
    glGenBuffers(1, &g_vertexDataVBO);
    glGenBuffers(1, &g_vertexDataEBO);

    glBindVertexArray(g_vertexDataVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexDataVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_vertexDataEBO);

    // Attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glBindVertexArray(0);

    // Weighted Vertex
    glGenVertexArrays(1, &g_weightedVertexDataVAO);
    glGenBuffers(1, &g_weightedVertexDataVBO);
    glGenBuffers(1, &g_weightedVertexDataEBO);

    glBindVertexArray(g_weightedVertexDataVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_weightedVertexDataVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_weightedVertexDataEBO);

    // Attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, normal));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, boneID));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, weight));

    glBindVertexArray(0);
}

void Moon::Rendering::OpenGLBackEnd::Cleanup() {
    glDeleteVertexArrays(1, &g_vertexDataVAO);
    glDeleteBuffers(1, &g_vertexDataVBO);
    glDeleteBuffers(1, &g_vertexDataEBO);
    g_vertexDataVAO = 0; g_vertexDataVBO = 0; g_vertexDataEBO = 0;
}

void Moon::Rendering::OpenGLBackEnd::UpdateVertexBufferData(const Vertex* data, size_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexDataVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data, GL_STATIC_DRAW);

    glBindVertexArray(g_vertexDataVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexDataVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glBindVertexArray(0);
}

void Moon::Rendering::OpenGLBackEnd::UpdateIndexBufferData(const uint32_t* data, size_t count) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_vertexDataEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);

    glBindVertexArray(g_vertexDataVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_vertexDataEBO);
    glBindVertexArray(0);
}

void Moon::Rendering::OpenGLBackEnd::UpdateWightedVertexData(const Rendering::WeightedVertex* data, size_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, g_weightedVertexDataVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data, GL_STATIC_DRAW);

    glBindVertexArray(g_weightedVertexDataVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_weightedVertexDataVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, normal));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, boneID));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Rendering::WeightedVertex), (void*)offsetof(Rendering::WeightedVertex, weight));
    glBindVertexArray(0);
}

void Moon::Rendering::OpenGLBackEnd::UpdateWightedIndexData(const uint32_t* data, size_t count) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_weightedVertexDataEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);

    glBindVertexArray(g_vertexDataVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_weightedVertexDataEBO);
    glBindVertexArray(0);
}

uint32_t Moon::Rendering::OpenGLBackEnd::GetVertexDataVAO() {
	return g_vertexDataVAO;
}
//uint32_t Moon::Rendering::OpenGLBackEnd::GetVertexDataVBO() {
//    return g_vertexDataVBO;
//}
//uint32_t Moon::Rendering::OpenGLBackEnd::GetVertexDataEBO() {
//    return g_vertexDataEBO;
//}

uint32_t Moon::Rendering::OpenGLBackEnd::GetWeightedVertexDataVAO() {
    return g_weightedVertexDataVAO;
}

uint32_t Moon::Rendering::OpenGLBackEnd::GetWeightedVertexDataVBO() {
    return g_weightedVertexDataVBO;
}

uint32_t Moon::Rendering::OpenGLBackEnd::GetWeightedVertexDataEBO() {
    return g_weightedVertexDataEBO;
}