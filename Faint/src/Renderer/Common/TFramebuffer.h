#pragma once

#include <memory>
#include <variant>
#include <string>
#include <Renderer/Common/TTexture.h>
#include <Renderer/Common/TRenderbuffer.h>
#include <Renderer/Enums/FramebufferAttachment.h>

namespace Moon::Rendering {

	template<class TextureContext, class TextureHandleContext, class RenderBufferContext>
	using TFramebufferAttachment = std::variant<
		std::shared_ptr<TTexture<TextureContext, TextureHandleContext>>,
		std::shared_ptr<TRenderbuffer<RenderBufferContext>>
	>;
		
	template<class FramebufferContext, class TextureContext, class TextureHandleContext, class RenderBufferContext>
	class TFramebuffer {
	public:
		using Attachment = TFramebufferAttachment<TextureContext, TextureHandleContext, RenderBufferContext>;

		template<typename T>
		static constexpr bool IsSupportedAttachmentType = 
			std::same_as<T, TTexture<TextureContext, TextureHandleContext>> ||
			std::same_as<T, TRenderbuffer<RenderBufferContext>>;
			
		TFramebuffer(std::string_view p_debugname = std::string_view{});

		~TFramebuffer();

		void Bind() const;

		void Unbind() const;

		/**
		* Validate the framebuffer. Must be executed at least once after the framebuffer creation.
		* @note It's recommended to call this method after each attachment change.
		* @return Returns true if the framebuffer has been validated successfully.
		*/
		bool Validate();

		bool IsValid();

		void Resize(uint16_t p_width, uint16_t p_height);

		template<class T> requires IsSupportedAttachmentType<T>
		void Attach(std::shared_ptr<T> p_toAttach, EFramebufferAttachment p_attachment, uint32_t p_index = 0, std::optional<uint32_t> p_layer = std::nullopt);
	
		template<class T> requires IsSupportedAttachmentType<T>
		std::optional<std::reference_wrapper<T>> GetAttachment(EFramebufferAttachment p_attachment, uint32_t p_index = 0) const;

		uint32_t GetID() const;

		const std::string& GetDebugName() const;

	protected:
		FramebufferContext m_context;
	};
}