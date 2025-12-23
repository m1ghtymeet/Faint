#pragma once

/* TODO: Change by Internal Format */
#include <Renderer/Enums/Format.h>

namespace Moon::Rendering {

	template<class Context>
	class TRenderbuffer final {
	public:
		/**
		* Create the render buffer
		*/
		TRenderbuffer();

		/**
		* Desctructor
		*/
		~TRenderbuffer();

		void Bind() const;

		void Unbind() const;

		uint32_t GetID() const;

		void Allocate(uint16_t p_width, uint16_t p_height);

		bool IsValid() const;

		void Resize(uint16_t p_width, uint16_t p_height);

		uint16_t GetWidth() const;

		uint16_t GetHeight() const;

	private:
		Context m_context;
	};
}