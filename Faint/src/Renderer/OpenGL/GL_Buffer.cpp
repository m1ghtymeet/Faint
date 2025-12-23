#include "GL_Buffer.h"
#include <Renderer/OpenGL/GL_Types.h>
#include <Debug/Assertion.h>
#include <glad/glad.h>

template<>
Moon::Rendering::OpenGLBuffer::TBuffer(BufferType p_type) : m_buffer{
	.type = p_type
} {
	glCreateBuffers(1, &m_buffer.id);
}

template<>
Moon::Rendering::OpenGLBuffer::~TBuffer() {
	glDeleteBuffers(1, &m_buffer.id);
}

template<>
uint64_t Moon::Rendering::OpenGLBuffer::Allocate(uint64_t p_size, EAccessSpecified p_usage) {
	FT_ASSERT(IsValid(), "Cannot allocate memory for an invalid buffer");
	glNamedBufferData(m_buffer.id, p_size, nullptr, EnumToValue<GLenum>(p_usage));
	return m_buffer.allocatedBytes = p_size;
}

template<>
void Moon::Rendering::OpenGLBuffer::Upload(const void* p_data, std::optional<BufferMemoryRange> p_range) {
	FT_ASSERT(IsValid(), "Trying to upload data to an invalid buffer");
	FT_ASSERT(!IsEmpty(), "Trying to upload data to an empty buffer");

	glNamedBufferSubData(
		m_buffer.id,
		p_range ? p_range->offset : 0,
		p_range ? p_range->size : m_buffer.allocatedBytes,
		p_data
	);
}

template<>
bool Moon::Rendering::OpenGLBuffer::IsValid() const {
	return m_buffer.id != 0 &&
		 m_buffer.type != BufferType::UNKNOWN;
}

template<>
bool Moon::Rendering::OpenGLBuffer::IsEmpty() const {
	return GetSize() == 0;
}

template<>
uint64_t Moon::Rendering::OpenGLBuffer::GetSize() const {
	FT_ASSERT(IsValid(), "Cannot get size of an invalid buffer");
	return m_buffer.allocatedBytes;
}

template<>
void Moon::Rendering::OpenGLBuffer::Bind(std::optional<uint32_t> p_index) const {
	FT_ASSERT(IsValid(), "Cannot bind an invalid buffer");
	if (p_index.has_value()) {
		glBindBufferBase(EnumToValue<GLenum>(m_buffer.type), p_index.value(), m_buffer.id);
	}
	else {
		glBindBuffer(EnumToValue<GLenum>(m_buffer.type), m_buffer.id);
	}
}

template<>
void Moon::Rendering::OpenGLBuffer::Unbind() const {
	FT_ASSERT(IsValid(), "Cannot unbind an invalid buffer");
	glBindBuffer(EnumToValue<GLenum>(m_buffer.type), 0);
}

template<>
uint32_t Moon::Rendering::OpenGLBuffer::GetID() const {
	FT_ASSERT(IsValid(), "Cannot get ID of an invalid buffer");
	return m_buffer.id;
}
