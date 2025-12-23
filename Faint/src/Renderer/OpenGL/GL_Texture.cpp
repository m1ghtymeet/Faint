#pragma once

#include <Renderer/OpenGL/GL_Texture.h>
#include <Renderer/OpenGL/GL_Types.h>
#include <Renderer/Enums/TextureType.h>
#include <Renderer/Types/TextureData.h>
#include <Debug/Assertion.h>

#include <glad/glad.h>

namespace Moon::Rendering::OpenGLUtil {
	GLint TextureWrapModeToGLEnum(TextureWrapMode wrapMode) {
		switch (wrapMode) {
		case TextureWrapMode::REPEAT: return GL_REPEAT;
		case TextureWrapMode::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case TextureWrapMode::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		case TextureWrapMode::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
		default: return GL_NONE;
		}
	}
	GLint TextureFilterToGLEnum(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::NEAREST: return GL_NEAREST;
		case TextureFilter::LINEAR: return GL_LINEAR;
		case TextureFilter::LINEAR_MIPMAP: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_NONE;
		}
	}
}

Moon::Rendering::OpenGLTexture::OpenGLTexture(Moon::Rendering::ETextureType p_type, std::string p_debugName) :
	m_type(EnumToValue<GLenum>(p_type))
{
	glCreateTextures(m_type, 1, &m_id);
	m_debugName = p_debugName;
}

Moon::Rendering::OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &m_id);
	//glDeleteBuffers(3, m_pbo.data());
}

uint32_t Moon::Rendering::OpenGLTexture::GetID() const {
	return m_id;
}

void Moon::Rendering::OpenGLTexture::Create(const Data::TextureData& p_data) {
	if (m_allocated)
		return;

	auto& data = m_data;

	data = p_data;
	data.width = std::max(1, m_data.width);
	data.height = std::max(1, m_data.height);

	m_mipmapLevelCount = m_data.useMipMaps
		? static_cast<int>(std::floor(std::log2(std::max(data.width, data.height))) + 1)
		: 1;

	// If the underlying texture is a cube map, this will alocate all 6 sides.
	// No need to iterate over each side.
	glTextureStorage2D(
		m_id,
		m_mipmapLevelCount,
		data.internalFormat,
		data.width,
		data.height
	);

	SetMinFilter(m_data.useMipMaps ? TextureFilter::LINEAR_MIPMAP : TextureFilter::LINEAR);
	SetMagFilter(TextureFilter::LINEAR);
	SetWrapMode(TextureWrapMode::REPEAT);

	//if (m_pbo && !m_pbo->IsValid()) {
	//	m_pbo->Init(data.width * data.height * 4); // Adjust multiplier based on format
	//}

	m_allocated = true;
}

void Moon::Rendering::OpenGLTexture::SetWrapMode(TextureWrapMode wrapMode) {
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, OpenGLUtil::TextureWrapModeToGLEnum(wrapMode));
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, OpenGLUtil::TextureWrapModeToGLEnum(wrapMode));
}

void Moon::Rendering::OpenGLTexture::SetMinFilter(TextureFilter filter) {
	glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, OpenGLUtil::TextureFilterToGLEnum(filter));
}

void Moon::Rendering::OpenGLTexture::SetMagFilter(TextureFilter filter) {
	glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, OpenGLUtil::TextureFilterToGLEnum(filter));
}

void Moon::Rendering::OpenGLTexture::MakeBindlessTextureResident() {
	// TODO: Make Bindless
}

void Moon::Rendering::OpenGLTexture::MakeBindlessTextureNonResident() {
	// TODO: Make Bindless
}

void Moon::Rendering::OpenGLTexture::Bind(std::optional<uint32_t> p_slot) const {
	if (p_slot.has_value()) {
		glBindTextureUnit(p_slot.value(), m_id);
	}
	else {
		glBindTexture(m_type, m_id);
	}
}

void Moon::Rendering::OpenGLTexture::Unbind() const {
	glBindTexture(m_type, 0);
}

bool Moon::Rendering::OpenGLTexture::IsValid() const {
	return m_allocated;
}

void Moon::Rendering::OpenGLTexture::Upload(const void* p_data, int p_format, EPixelDataType p_type) {
	FT_CORE_ASSERT(m_allocated, "Cannot upload data to a texture before it has been allocated");
	FT_CORE_ASSERT(IsValid(), "Cannot upload data to a texture before it has been allocated");
	FT_CORE_ASSERT(p_data, "Cannot upload texture data from a null pointer");

	int mipmapWidth = m_data.width;
	int mipmapHeight = m_data.height;
	if (m_data.imageDataType == Data::ImageDataType::UNCOMPRESSED) {
		glTextureSubImage2D(m_id, 0, 0, 0, mipmapWidth, mipmapHeight,
			p_format, EnumToValue<GLenum>(p_type), p_data);
	}
	if (m_data.imageDataType == Data::ImageDataType::COMPRESSED) {
		glCompressedTextureSubImage2D(m_id, 0, 0, 0, mipmapWidth, mipmapHeight,
			m_data.internalFormat, m_data.dataSize, p_data);
	}

	if (m_data.useMipMaps) glGenerateTextureMipmap(m_id);
}

void Moon::Rendering::OpenGLTexture::Upload(const uint32_t& pbo, int format, EPixelDataType p_type) {
	FT_CORE_ASSERT(m_allocated, "Cannot upload data to a texture before it has been allocated");
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

	if (m_data.imageDataType == Data::ImageDataType::UNCOMPRESSED) {
		glTextureSubImage2D(m_id, 0, 0, 0, m_data.width, m_data.height,
			format, EnumToValue<GLenum>(p_type), nullptr);
	}
	if (m_data.imageDataType == Data::ImageDataType::COMPRESSED) {
		glCompressedTextureSubImage2D(m_id, 0, 0, 0, m_data.width, m_data.height,
			m_data.internalFormat, m_data.dataSize, nullptr);
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

GLubyte* Moon::Rendering::OpenGLTexture::GetPBOBuffer(size_t requiredSize) {
	FT_CORE_ASSERT(m_allocated, "Texture must be allocated before getting PBO buffer");
	FT_CORE_ASSERT(m_pbo, "PBO not initialized");

	if (!m_pbo->IsValid() || m_pbo->GetPersistentBuffer() == nullptr) {
		m_pbo->Init(requiredSize);
	}

	return m_pbo->GetPersistentBuffer();
}

void Moon::Rendering::OpenGLTexture::UploadAsync(const void* p_data, size_t dataSize, int format, EPixelDataType p_type) {
	FT_CORE_ASSERT(m_allocated, "Cannot upload data to a texture before it has been allocated");
	FT_CORE_ASSERT(m_pbo, "PBO not initialized");
	FT_CORE_ASSERT(p_data, "Cannot upload texture data from a null pointer");

	// Get PBO buffer and copy data
	GLubyte* buffer = GetPBOBuffer(dataSize);
	std::memcpy(buffer, p_data, dataSize);

	// Start async upload
	BeginAsyncUpload(format, p_type);
}

void Moon::Rendering::OpenGLTexture::BeginAsyncUpload(int format, EPixelDataType p_type) {
	FT_CORE_ASSERT(m_allocated, "Texture must be allocated");
	FT_CORE_ASSERT(m_pbo, "PBO not initialized");

	m_pbo->BeginUpload();
	m_lastUploadFormat = format;
	m_lastUploadDataType = p_type;
	m_asyncUploadInProgress = true;

	// Perform the upload
	Upload(m_pbo->GetHandle(), format, p_type);
}

bool Moon::Rendering::OpenGLTexture::UpdateAsyncUpload() {
	if (!m_asyncUploadInProgress || !m_pbo)
		return false;

	bool inProgress = m_pbo->UpdateState();
	if (!inProgress) {
		m_asyncUploadInProgress = false;
	}

	return inProgress;
}

bool Moon::Rendering::OpenGLTexture::IsAsyncUploadComplete() const {
	return !m_asyncUploadInProgress;
}

void Moon::Rendering::OpenGLTexture::WaitForAsyncUpload() {
	if (m_asyncUploadInProgress && m_pbo) {
		m_pbo->WaitOnFence();
		m_asyncUploadInProgress = false;
	}
}

const Moon::Rendering::Data::TextureData& Moon::Rendering::OpenGLTexture::GetData() const {
	FT_CORE_ASSERT(IsValid(), "Cannot get the descriptor of a texture before it has been allocated");
	return m_data;
}

void Moon::Rendering::OpenGLTexture::GenerateMipmaps() const {
	FT_CORE_ASSERT(IsValid(), "Cannot generate mipmaps for a texture before it has been allocated");
	FT_CORE_ASSERT(m_data.useMipMaps, "Cannot generate mipmaps for a texture that doesn't use them");

	glGenerateTextureMipmap(m_id);
}

