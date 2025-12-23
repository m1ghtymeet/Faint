#pragma once

#include <Renderer/OpenGL/GL_Texture.h>
#include <Renderer/OpenGL/GL_Types.h>
#include <Renderer/Enums/TextureType.h>
/*
Moon::Rendering::OpenGLTextureHandle::OpenGLTextureHandle(ETextureType p_type) :
	m_type(EnumToValue<GLenum>(p_type))
{
}

Moon::Rendering::OpenGLTextureHandle::OpenGLTextureHandle(ETextureType p_type, uint32_t p_id) :
	m_id(p_id),
	m_type(EnumToValue<GLenum>(p_type))
{
}

void Moon::Rendering::OpenGLTextureHandle::Bind(std::optional<uint32_t> p_slot) const {
	if (p_slot.has_value()) {
		glBindTextureUnit(p_slot.value(), m_id);
	}
	else {
		glBindTexture(m_type, m_id);
	}
}

void Moon::Rendering::OpenGLTextureHandle::Unbind() const {
	glBindTexture(m_type, 0);
}

uint32_t Moon::Rendering::OpenGLTextureHandle::GetID() const {
	return m_id;
}*/