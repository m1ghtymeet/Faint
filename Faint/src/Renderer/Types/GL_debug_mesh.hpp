#pragma once
#include <glad/glad.h>
#include "Common/Types.h"

struct OpenGLDebugMesh {
private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_allocatedBufferSize = 0;
    unsigned int m_vertexCount = 0;

public:
    size_t GetVertexCount() const { return m_vertexCount; }
    unsigned int GetVAO() const { return m_vao; }

    void UpdateVertexData(std::vector<DebugVertex>& vertices) {
        // Create VAO/VBO if they don't exist
        if (m_vao == 0) {
            glGenVertexArrays(1, &m_vao);
            glGenBuffers(1, &m_vbo);
        }

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        size_t bufferSize = vertices.size() * sizeof(DebugVertex);

        // Allocate or resize buffer if necessary
        if (bufferSize > m_allocatedBufferSize) {
            glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
            m_allocatedBufferSize = bufferSize;
        }

        // Upload new vertex data
        glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, vertices.data());

        // Set up vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));

        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(2, 2, GL_INT, sizeof(DebugVertex), (void*)offsetof(DebugVertex, pixelOffset));
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vertexCount = vertices.size();
    }
};