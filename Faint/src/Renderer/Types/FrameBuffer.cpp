#include "FrameBuffer.h"

GLenum GLInternalFormatToGLType(GLenum internalFormat) {
    switch (internalFormat) {
        // 8-bit unsigned and signed integer formats
    case GL_R8UI:      return GL_UNSIGNED_BYTE;
    case GL_R8I:       return GL_BYTE;
    case GL_RG8UI:     return GL_UNSIGNED_BYTE;
    case GL_RG8I:      return GL_BYTE;
    case GL_RGBA8UI:   return GL_UNSIGNED_BYTE;
    case GL_RGBA8I:    return GL_BYTE;

        // 16-bit unsigned and signed integer formats
    case GL_R16UI:     return GL_UNSIGNED_SHORT;
    case GL_R16I:      return GL_SHORT;
    case GL_RG16UI:    return GL_UNSIGNED_SHORT;
    case GL_RG16I:     return GL_SHORT;
    case GL_RGBA16UI:  return GL_UNSIGNED_SHORT;
    case GL_RGBA16I:   return GL_SHORT;

        // 32-bit unsigned and signed integer formats
    case GL_R32UI:     return GL_UNSIGNED_INT;
    case GL_R32I:      return GL_INT;
    case GL_RG32UI:    return GL_UNSIGNED_INT;
    case GL_RG32I:     return GL_INT;
    case GL_RGBA32UI:  return GL_UNSIGNED_INT;
    case GL_RGBA32I:   return GL_INT;

        // Special packed integer format
    case GL_RGB10_A2UI: return GL_UNSIGNED_INT_2_10_10_10_REV;

        // Normalized unsigned formats (non-integer)
    case GL_R8:        return GL_UNSIGNED_BYTE;
    case GL_RG8:       return GL_UNSIGNED_BYTE;
    case GL_RGBA8:     return GL_UNSIGNED_BYTE;
    case GL_SRGB8:     return GL_UNSIGNED_BYTE;
    case GL_SRGB8_ALPHA8: return GL_UNSIGNED_BYTE;
    case GL_R16:       return GL_UNSIGNED_SHORT;
    case GL_RG16:      return GL_UNSIGNED_SHORT;
    case GL_RGBA16:    return GL_UNSIGNED_SHORT;

        // Floating point formats
    case GL_R16F:      return GL_FLOAT;
    case GL_RG16F:     return GL_FLOAT;
    case GL_RGB16F:   return GL_FLOAT;
    case GL_RGBA16F:   return GL_FLOAT;
    case GL_R32F:      return GL_FLOAT;
    case GL_RG32F:     return GL_FLOAT;
    case GL_RGBA32F:   return GL_FLOAT;
        // Depth and depth-stencil formats (if needed)
        // case GL_DEPTH_COMPONENT16:       return GL_UNSIGNED_SHORT;
        // case GL_DEPTH_COMPONENT24:       return GL_UNSIGNED_INT;
        // case GL_DEPTH_COMPONENT32F:      return GL_FLOAT;
        // case GL_DEPTH24_STENCIL8:        return GL_UNSIGNED_INT_24_8;
        // case GL_DEPTH32F_STENCIL8:       return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
    default:
        std::cout << "OpenGLUtil::InternalFormatToType(GLenum internalFormat) failed Unsupported internal format\n";
        return 0;
    }
}

GLenum GLInternalFormatToGLFormat(GLenum internalFormat) {
    switch (internalFormat) {
        // Red channel formats
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
        return GL_RED;
    case GL_R8UI:
    case GL_R8I:
    case GL_R16UI:
    case GL_R16I:
    case GL_R32UI:
    case GL_R32I:
        return GL_RED_INTEGER;

        // Red-Green channel formats
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
        return GL_RG;
    case GL_RG8UI:
    case GL_RG8I:
    case GL_RG16UI:
    case GL_RG16I:
    case GL_RG32UI:
    case GL_RG32I:
        return GL_RG_INTEGER;

        // RGB channel formats
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_SRGB8:
        return GL_RGB;
    case GL_RGB8UI:
    case GL_RGB8I:
    case GL_RGB16UI:
    case GL_RGB16I:
    case GL_RGB32UI:
    case GL_RGB32I:
        return GL_RGB_INTEGER;

        // RGBA channel formats
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGBA16:
    case GL_RGBA16_SNORM:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_SRGB8_ALPHA8:
        return GL_RGBA;
    case GL_RGBA8UI:
    case GL_RGBA8I:
    case GL_RGBA16UI:
    case GL_RGBA16I:
    case GL_RGBA32UI:
    case GL_RGBA32I:
        return GL_RGBA_INTEGER;

        // Special packed formats
    case GL_RGB10_A2:
    case GL_RGB10_A2UI:
        return GL_RGBA;

        // Depth formats
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32F:
        return GL_DEPTH_COMPONENT;

        // Depth-stencil formats
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return GL_DEPTH_STENCIL;

    default:
        std::cout << "GLInternalFormatToGLFormat: Unsupported internal format\n";
        return 0;
    }
}

namespace Faint {
    void FrameBuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    }

    void FrameBuffer::SetViewport() {
        glViewport(0, 0, m_width, m_height);
    }

    FrameBuffer::FrameBuffer(const char* name, int width, int height) {
        Create(name, width, height);
    }

    FrameBuffer::FrameBuffer(const char* name, const glm::ivec2& size) {
        Create(name, size);
    }

    void FrameBuffer::Create(const char* name, int width, int height) {
        glCreateFramebuffers(1, &m_id);
        this->m_name = name;
        this->m_width = width;
        this->m_height = height;
    }

    void FrameBuffer::CleanUp() {
        m_colorAttachments.clear();
        glDeleteFramebuffers(1, &m_id);
        m_id = 0;
    }

    void FrameBuffer::Create(const char* name, const glm::ivec2& size) {
        Create(name, size.x, size.y);
    }

    void FrameBuffer::CreateAttachment(const char* name, GLenum internalFormat, GLenum minFilter, GLenum magFilter) {
        ColorAttachment& colorAttachment = m_colorAttachments.emplace_back();
        colorAttachment.name = name;
        colorAttachment.internalFormat = internalFormat;
        colorAttachment.format = GLInternalFormatToGLFormat(internalFormat);
        colorAttachment.type = GLInternalFormatToGLType(internalFormat);
        glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment.id);
        glTextureStorage2D(colorAttachment.id, 1, internalFormat, m_width, m_height);
        glTextureParameteri(colorAttachment.id, GL_TEXTURE_MIN_FILTER, minFilter);
        glTextureParameteri(colorAttachment.id, GL_TEXTURE_MAG_FILTER, magFilter);
        glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (internalFormat == GL_R32I)
            glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glNamedFramebufferTexture(m_id, GL_COLOR_ATTACHMENT0 + m_colorAttachments.size() - 1, colorAttachment.id, 0);
        std::string debugLabel = "Texture (FBO: " + std::string(m_name) + " Tex: " + std::string(name) + ")";
        glObjectLabel(GL_TEXTURE, colorAttachment.id, static_cast<GLsizei>(debugLabel.length()), debugLabel.c_str());
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Framebuffer is not complete: " << name << std::endl;
    }


    void FrameBuffer::CreateDepthAttachment(GLenum internalFormat, GLenum minFilter, GLenum magFilter, GLint wrap, glm::vec4 borderColor) {
        m_depthAttachment.internalFormat = internalFormat;
        glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment.id);
        glTextureStorage2D(m_depthAttachment.id, 1, internalFormat, m_width, m_height);
        glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_MIN_FILTER, minFilter);
        glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_MAG_FILTER, magFilter);
        glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, m_depthAttachment.id, 0);
        std::string debugLabel = "Texture (FBO: " + std::string(m_name) + " Tex: Depth)";
        glObjectLabel(GL_TEXTURE, m_depthAttachment.id, static_cast<GLsizei>(debugLabel.length()), debugLabel.c_str());
    }

    void FrameBuffer::BindDepthAttachmentFrom(const FrameBuffer& srcFrameBuffer) {
        glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, srcFrameBuffer.m_depthAttachment.id, 0);
    }

    bool FrameBuffer::StrCmp(const char* queryA, const char* queryB) {
        return std::strcmp(queryA, queryB) == 0;
    }

    void FrameBuffer::DrawBuffers(std::vector<const char*> attachmentNames) {
        std::vector<GLuint> attachments;
        for (const char* attachmentName : attachmentNames) {
            attachments.push_back(GetColorAttachmentSlotByName(attachmentName));
        }
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    }

    void FrameBuffer::DrawBuffer(const char* attachmentName) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachment(const char* attachmentName, GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLfloat clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, 0, 0, 0, GetWidth(), GetHeight(), 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachmentI(const char* attachmentName, GLint r, GLint g, GLint b, GLint a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLint clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, 0, 0, 0, GetWidth(), GetHeight(), 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachmentUI(const char* attachmentName, GLint r, GLint g, GLint b, GLint a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLuint clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, 0, 0, 0, GetWidth(), GetHeight(), 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachmenSubRegion(const char* attachmentName, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height, GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLfloat clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, xOffset, yOffset, 0, width, height, 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachmenSubRegionInt(const char* attachmentName, GLint xOffset, GLint yOffset, GLsizei width, GLsizei  height, GLint r, GLint g, GLint b, GLint a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLuint clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, xOffset, yOffset, 0, width, height, 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearAttachmenSubRegionUInt(const char* attachmentName, GLint xOffset, GLint yOffset, GLsizei width, GLsizei  height, GLuint r, GLuint g, GLuint b, GLuint a) {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(attachmentName, m_colorAttachments[i].name)) {
                GLuint texture = m_colorAttachments[i].id;
                GLenum internalFormat = m_colorAttachments[i].internalFormat;
                GLenum format = m_colorAttachments[i].format;
                GLenum type = m_colorAttachments[i].type;
                GLuint clearColor[4] = { r, g, b, a };
                glClearTexSubImage(texture, 0, xOffset, yOffset, 0, width, height, 1, format, type, clearColor);
                return;
            }
        }
    }

    void FrameBuffer::ClearDepthAttachment() {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void FrameBuffer::Resize(int width, int height) {
        m_width = std::max(width, 1);
        m_height = std::max(height, 1);

        for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
            ColorAttachment& colorAttachment = m_colorAttachments[i];
            glDeleteTextures(1, &colorAttachment.id);
            glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment.id);
            glTextureStorage2D(colorAttachment.id, 1, colorAttachment.internalFormat, m_width, m_height);
            glTextureParameteri(colorAttachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(colorAttachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (colorAttachment.internalFormat == GL_R32I)
                glTextureParameteri(colorAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glNamedFramebufferTexture(m_id, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), colorAttachment.id, 0);
            std::string debugLabel = "Texture (FBO: " + std::string(m_name) + " Tex: " + std::string(colorAttachment.name) + ")";
            glObjectLabel(GL_TEXTURE, colorAttachment.id, static_cast<GLsizei>(debugLabel.length()), debugLabel.c_str());
            
        }

        if (m_depthAttachment.id != 0) {
            glDeleteTextures(1, &m_depthAttachment.id);
            glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment.id);
            glTextureStorage2D(m_depthAttachment.id, 1, m_depthAttachment.internalFormat, m_width, m_height);
            glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_depthAttachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, m_depthAttachment.id, 0);
            std::string debugLabel = "Texture (FBO: " + std::string(m_name) + " Tex: Depth)";
            glObjectLabel(GL_TEXTURE, m_depthAttachment.id, static_cast<GLsizei>(debugLabel.length()), debugLabel.c_str());
        }
        //std::cout << "Resized '" << m_name << "' framebuffer to " << m_width << ", " << m_height << "\n";
    }

    GLuint FrameBuffer::GetHandle() const {
        return m_id;
    }

    GLuint FrameBuffer::GetWidth() const {
        return m_width;
    }

    GLuint FrameBuffer::GetHeight() const {
        return m_height;
    }

    GLuint FrameBuffer::GetColorAttachmentHandleByName(const char* name) const {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(name, m_colorAttachments[i].name)) {
                return m_colorAttachments[i].id;
            }
        }
        std::cerr << "GetColorAttachmentHandleByName() with name '" << name << "' failed. Name does not exist in FrameBuffer '" << this->m_name << "'\n";
        return GL_NONE;
    }

    GLuint FrameBuffer::GetDepthAttachmentHandle() const {
        return m_depthAttachment.id;
    }

    GLenum FrameBuffer::GetColorAttachmentSlotByName(const char* name) const {
        for (int i = 0; i < m_colorAttachments.size(); i++) {
            if (StrCmp(name, m_colorAttachments[i].name)) {
                return GL_COLOR_ATTACHMENT0 + i;
            }
        }
        std::cerr << "GetColorAttachmentSlotByName() with name '" << name << "' failed. Name does not exist in FrameBuffer '" << this->m_name << "'\n";
        return GL_INVALID_VALUE;
    }

    void FrameBuffer::BlitToDefaultFrameBuffer(const char* srcName, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, GetHandle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glReadBuffer(GetColorAttachmentSlotByName(srcName));
        glDrawBuffer(GL_BACK);
        glBlitFramebuffer(0, 0, GetWidth(), GetHeight(), dstX0, dstY0, dstX1, dstY1, mask, filter);
    }
}