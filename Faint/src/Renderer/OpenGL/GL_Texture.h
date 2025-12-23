#pragma once

#include <Renderer/Enums/TextureType.h>
#include <Renderer/Types/TextureData.h>
#include <Renderer/OpenGL/GL_PBO.hpp>
#include <string>
#include <array>
#include <memory>

namespace Moon::Rendering {

	enum class TextureWrapMode {
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	enum class TextureFilter {
		LINEAR,
		NEAREST,
		LINEAR_MIPMAP
	};

	class OpenGLTexture {
	public:
		OpenGLTexture(ETextureType p_type, std::string p_debugName = "");
		~OpenGLTexture();
		uint32_t GetID() const;
		void Create(const Data::TextureData& p_data);
		void SetWrapMode(TextureWrapMode wrapMode);
		void SetMinFilter(TextureFilter filter);
		void SetMagFilter(TextureFilter filter);
		void MakeBindlessTextureResident();
		void MakeBindlessTextureNonResident();
		void Bind(std::optional<uint32_t> p_slot = std::nullopt) const;
		void Unbind() const;
		bool IsValid() const;

		void Upload(const void* p_data, /*EFormat p_format*/int format, EPixelDataType p_type = EPixelDataType::UNSIGNED_BYTE);
		void Upload(const uint32_t& pbo, int format, EPixelDataType p_type = EPixelDataType::UNSIGNED_BYTE);
		
		void UploadAsync(const void* p_data, size_t dataSize, int format, EPixelDataType p_type = EPixelDataType::UNSIGNED_BYTE);
		bool UpdateAsyncUpload();
		GLubyte* GetPBOBuffer(size_t requiredSize);
		void BeginAsyncUpload(int format, EPixelDataType p_type = EPixelDataType::UNSIGNED_BYTE);
		bool IsAsyncUploadComplete() const;
		void WaitForAsyncUpload();

		const Data::TextureData& GetData() const;
		void GenerateMipmaps() const;

	private:
		uint32_t m_id = 0;
		const uint32_t m_type;
		int m_mipmapLevelCount = 0;
		std::string m_debugName;
		bool m_allocated = false;
		bool m_mipmapsGenerated = false;
		Data::TextureData m_data;

		// PBO for async uploads
		std::unique_ptr<PBO> m_pbo;
		bool m_asyncUploadInProgress = false;
		int m_lastUploadFormat = 0;
		EPixelDataType m_lastUploadDataType = EPixelDataType::UNSIGNED_BYTE;
	};
}