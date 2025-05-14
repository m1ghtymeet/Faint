#include "hzpch.h"
#include "OpenGLVertexArray.h"

namespace Faint {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {

		switch (type) {
		case ShaderDataType::FLOAT:   return GL_FLOAT;
		case ShaderDataType::FLOAT2:  return GL_FLOAT;
		case ShaderDataType::FLOAT3:  return GL_FLOAT;
		case ShaderDataType::FLOAT4:  return GL_FLOAT;
		case ShaderDataType::MAT3:    return GL_FLOAT;
		case ShaderDataType::MAT4:    return GL_FLOAT;
		case ShaderDataType::INT:     return GL_INT;
		case ShaderDataType::INT2:     return GL_INT;
		case ShaderDataType::INT3:     return GL_INT;
		case ShaderDataType::INT4:     return GL_INT;
		case ShaderDataType::BOOL:     return GL_BOOL;
		}
		HZ_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray() {

		HZ_PROFILE_FUNCTION();

		if (m_ID)
			glDeleteVertexArrays(1, &m_ID);

		glGenVertexArrays(1, &m_ID);
	}
	OpenGLVertexArray::~OpenGLVertexArray() {

		HZ_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_ID);
	}
	void OpenGLVertexArray::Bind() const {

		HZ_PROFILE_FUNCTION();

		glBindVertexArray(m_ID);
	}
	void OpenGLVertexArray::UnBind() const {

		HZ_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}
	GLuint OpenGLVertexArray::GetID() const
	{
		return m_ID;
	}
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {

		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			switch (element.type) {
			case ShaderDataType::FLOAT: 
			case ShaderDataType::FLOAT2:
			case ShaderDataType::FLOAT3:
			case ShaderDataType::FLOAT4: 
				glEnableVertexAttribArray(m_vertexBufferIndex);
				glVertexAttribPointer(m_vertexBufferIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					sizeof(Vertex),
					(const void*)element.offset);
				m_vertexBufferIndex++; 
				break;
			case ShaderDataType::INT:
			case ShaderDataType::INT2:
			case ShaderDataType::INT3:
			case ShaderDataType::INT4:
			case ShaderDataType::BOOL:
				glEnableVertexAttribArray(m_vertexBufferIndex);
				glVertexAttribIPointer(m_vertexBufferIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.type),
					sizeof(Vertex),
					(const void*)element.offset);
				m_vertexBufferIndex++;
				break;
			case ShaderDataType::MAT3:
			case ShaderDataType::MAT4:
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++) {
					glEnableVertexAttribArray(m_vertexBufferIndex);
					glVertexAttribPointer(m_vertexBufferIndex,
						count,
						ShaderDataTypeToOpenGLBaseType(element.type),
						element.normalized ? GL_TRUE : GL_FALSE,
						sizeof(Vertex),
						(const void*)element.offset);
					glVertexAttribDivisor(m_vertexBufferIndex, 1);
					m_vertexBufferIndex++;
				}
				break;
			
			}
		}

		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		m_vertexBuffers.push_back(vertexBuffer);
	}
	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {

		glBindVertexArray(m_ID);
		indexBuffer->Bind();

		m_indexBuffers = indexBuffer;
	}
	const std::vector<Ref<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffers() const {
		return m_vertexBuffers;
	}
	const Ref<IndexBuffer>& OpenGLVertexArray::GetIndexBuffer() const {
		return m_indexBuffers;
	}
}