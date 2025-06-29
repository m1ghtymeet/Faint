#include "hzpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Faint {

	/* -------------------------------= Vertices =------------------------------- */
	OpenGLVertexBuffer::OpenGLVertexBuffer(std::vector<Vertex>& vertices) {
		if (m_ID)
			glDeleteBuffers(1, &m_ID);

		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLVertexBuffer::Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void OpenGLVertexBuffer::UnBind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/* -------------------------------= Indices =------------------------------- */
	OpenGLIndexBuffer::OpenGLIndexBuffer(std::vector<uint32_t> indices)
		: m_count((uint32_t)indices.size()) {
		if (m_ID)
			glDeleteBuffers(1, &m_ID);

		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLIndexBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}

	void OpenGLIndexBuffer::UnBind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
