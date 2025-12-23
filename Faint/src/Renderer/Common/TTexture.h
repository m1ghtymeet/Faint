#pragma once

#include <optional>
#include <string>
#include <Renderer/Enums/TextureType.h>
#include <Renderer/Enums/Format.h>
#include <Renderer/Types/TextureData.h>

namespace Moon::Rendering {

	template<class Context>
	class TTextureHandle {
	public:
		void Bind(std::optional<uint32_t> p_slot = std::nullopt) const;

		void Unbind() const;

		uint32_t GetID() const;

	protected:
		TTextureHandle(ETextureType p_type);
		TTextureHandle(ETextureType p_type, uint32_t p_id);

	protected:
		Context m_context;
	};
	
	template<class TextureContext, class TextureHandleContext>
	class TTexture final : public TTextureHandle<TextureHandleContext> {
	public:
		TTexture(ETextureType p_type, std::string p_debugName = "");

		~TTexture();

		void Allocate(const Data::TextureData& p_data);

		bool IsValid() const;

		void Upload(const void* p_data, EFormat p_format);

		const Data::TextureData& GetData() const;

		void GenerateMipmaps() const;

	private:
		TextureContext m_textureContext;
	};
}