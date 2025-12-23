#include "GL_VertexArray.h"

#include <Debug/Assertion.h>
#include <Renderer/OpenGL/GL_Types.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <Renderer/Types/Vertex.h>

namespace {
	uint32_t GetDataTypeSizeInBytes(Moon::Rendering::EDataType p_type) {
		switch (p_type) {
		case Moon::Rendering::EDataType::BYTE: return sizeof(GLbyte);
		case Moon::Rendering::EDataType::UNSIGNED_BYTE: return sizeof(GLubyte);
		case Moon::Rendering::EDataType::SHORT: return sizeof(GLshort);
		case Moon::Rendering::EDataType::UNSIGNED_SHORT: return sizeof(GLushort);
		case Moon::Rendering::EDataType::INT: return sizeof(GLint);
		case Moon::Rendering::EDataType::UNSIGNED_INT: return sizeof(GLuint);
		case Moon::Rendering::EDataType::FLOAT: return sizeof(GLfloat);
		case Moon::Rendering::EDataType::DOUBLE: return sizeof(GLdouble);
		default: return 0;
		}
	}

	uint32_t GetDataGlmSizeInByte(int p_count) {
		switch (p_count) {
		case 2: return sizeof(glm::vec2);
		case 3: return sizeof(glm::vec3);
		case 4: return sizeof(glm::vec4);
		default: return 0;
		}
	}

	uint32_t CalculateTotalVertexSize(std::span<const Moon::Rendering::VertexAttribute> p_attributes) {
		uint32_t result = 0;
		for (const auto& attribute : p_attributes) {
			result += GetDataTypeSizeInBytes(attribute.type) * attribute.count;
		}
		return result;
	}
}

template<>
Moon::Rendering::OpenGLVertexArray::TVertexArray() {
	glCreateVertexArrays(1, &m_context.id);
}

template<>
Moon::Rendering::OpenGLVertexArray::~TVertexArray() {
	glDeleteVertexArrays(1, &m_context.id);
}

template<>
void Moon::Rendering::OpenGLVertexArray::ApplyLayout(VertexAttributeLayout p_attributes, VertexBuffer& vb, IndexBuffer& ib) {
	FT_ASSERT(!IsValid(), "Vertex array layout already set");

	Bind();
	ib.Bind();
	vb.Bind();

	uint32_t attributeIndex = 0;

	const uint32_t totalSize = CalculateTotalVertexSize(p_attributes);
	intptr_t currentOffset = 0;

	//for (const auto& attribute : p_attributes) {
	//	FT_ASSERT(attribute.count >= 1 && attribute.count <= 4, "Attribute count must be between 1 and 4");
	//	
	//	glEnableVertexAttribArray(attributeIndex);
	//	glVertexAttribPointer(
	//		static_cast<GLuint>(attributeIndex),
	//		static_cast<GLuint>(attribute.count),
	//		EnumToValue<GLenum>(attribute.type),
	//		static_cast<GLboolean>(attribute.normalized),
	//		static_cast<GLsizei>(totalSize),
	//		reinterpret_cast<const GLvoid*>(currentOffset * GetDataGlmSizeInByte(attribute.count))
	//	);
	//
	//	//const uint64_t typeSize = GetDataTypeSizeInBytes(attribute.type);
	//	const uint64_t typeSize = GetDataGlmSizeInByte(attribute.count);
	//	const uint64_t attributeSize = attribute.count;
	//	currentOffset += attributeSize;
	//	attributeIndex++;
	//	m_context.attributeCount++;
	//}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Rendering::Vertex),
		(void*)offsetof(Rendering::Vertex, position)
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Rendering::Vertex),
		(void*)offsetof(Rendering::Vertex, uv)
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Rendering::Vertex),
		(void*)offsetof(Rendering::Vertex, normal)
	);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Rendering::Vertex),
		(void*)offsetof(Rendering::Vertex, tangent)
	);

	Unbind();
	ib.Unbind();
	vb.Unbind();
}

template<>
void Moon::Rendering::OpenGLVertexArray::ResetLayout() {
	FT_ASSERT(IsValid(), "Vertex array layout not already set");

	Bind();
	for (uint32_t i = 0; i < m_context.attributeCount; i++) {
		glDisableVertexAttribArray(i);
	}
	m_context.attributeCount = 0;
	Unbind();
}

template<>
void Moon::Rendering::OpenGLVertexArray::Bind() const {
	glBindVertexArray(m_context.id);
}

template<>
void Moon::Rendering::OpenGLVertexArray::Unbind() const {
	glBindVertexArray(0);
}

template<>
bool Moon::Rendering::OpenGLVertexArray::IsValid() const {
	return m_context.attributeCount > 0;
}

template<>
uint32_t Moon::Rendering::OpenGLVertexArray::GetID() const {
	return m_context.id;
}