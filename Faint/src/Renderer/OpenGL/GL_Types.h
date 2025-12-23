#pragma once

#include <Renderer/Common/Types.h>
#include <Renderer/Enums/AccessSpecified.h>
#include <Renderer/Enums/Format.h>
#include <Renderer/Enums/PixelDataType.h>
#include <Renderer/Enums/TextureType.h>
#include <Renderer/Enums/FramebufferAttachment.h>

#include <glad/glad.h>

namespace Moon::Rendering {

	template<typename ValueType, typename EnumType>
	constexpr ValueType EnumToValue(EnumType enumValue) {
		return Utils::ToValueImpl<EnumType, ValueType>(enumValue);
	}

	template<typename EnumType, typename ValueType>
	constexpr EnumType ValueToEnum(ValueType value) {
		return static_cast<EnumType>(value);
	}
}

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::EFramebufferAttachment, GLenum> {
	using EnumType = Moon::Rendering::EFramebufferAttachment;
	using type = std::tuple<
		EnumValuePair<EnumType::COLOR, GL_COLOR_ATTACHMENT0>,
		EnumValuePair<EnumType::DEPTH, GL_DEPTH_ATTACHMENT>,
		EnumValuePair<EnumType::STENCIL, GL_STENCIL_ATTACHMENT>,
		EnumValuePair<EnumType::DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::EDataType, GLenum> {
	using EnumType = Moon::Rendering::EDataType;
	using type = std::tuple<
		EnumValuePair<EnumType::FLOAT, GL_FLOAT>,
		EnumValuePair<EnumType::INT, GL_INT>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::BufferType, GLenum> {
	using EnumType = Moon::Rendering::BufferType;
	using type = std::tuple<
		EnumValuePair<EnumType::VERTEX, GL_ARRAY_BUFFER>,
		EnumValuePair<EnumType::INDEX, GL_ELEMENT_ARRAY_BUFFER>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::EAccessSpecified, GLenum> {
	using EnumType = Moon::Rendering::EAccessSpecified;
	using type = std::tuple<
		EnumValuePair<EnumType::STREAM_DRAW, GL_STREAM_DRAW>,
		EnumValuePair<EnumType::STREAM_READ, GL_STREAM_READ>,
		EnumValuePair<EnumType::STREAM_COPY, GL_STREAM_COPY>,
		EnumValuePair<EnumType::DYNAMIC_DRAW, GL_DYNAMIC_DRAW>,
		EnumValuePair<EnumType::DYNAMIC_READ, GL_DYNAMIC_READ>,
		EnumValuePair<EnumType::DYNAMIC_COPY, GL_DYNAMIC_COPY>,
		EnumValuePair<EnumType::STATIC_DRAW, GL_STATIC_DRAW>,
		EnumValuePair<EnumType::STATIC_READ, GL_STATIC_READ>,
		EnumValuePair<EnumType::STATIC_COPY, GL_STATIC_COPY>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::EFormat, GLenum> {
	using EnumType = Moon::Rendering::EFormat;
	using type = std::tuple<
		EnumValuePair<EnumType::RED, GL_RED>,
		EnumValuePair<EnumType::RG, GL_RG>,
		EnumValuePair<EnumType::RGB, GL_RGB>,
		EnumValuePair<EnumType::BGR, GL_BGR>,
		EnumValuePair<EnumType::RGBA, GL_RGBA>,
		EnumValuePair<EnumType::BGRA, GL_BGRA>,
		EnumValuePair<EnumType::RED_INTEGER, GL_RED_INTEGER>,
		EnumValuePair<EnumType::RG_INTEGER, GL_RG_INTEGER>,
		EnumValuePair<EnumType::RGB_INTEGER, GL_RGB_INTEGER>,
		EnumValuePair<EnumType::BGR_INTEGER, GL_BGR_INTEGER>,
		EnumValuePair<EnumType::RGBA_INTEGER, GL_RGBA_INTEGER>,
		EnumValuePair<EnumType::BGRA_INTEGER, GL_BGRA_INTEGER>,
		EnumValuePair<EnumType::STENCIL_INDEX, GL_STENCIL_INDEX>,
		EnumValuePair<EnumType::DEPTH_COMPONENT, GL_DEPTH_COMPONENT>,
		EnumValuePair<EnumType::DEPTH_STENCIL, GL_DEPTH_STENCIL>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::EPixelDataType, GLenum> {
	using EnumType = Moon::Rendering::EPixelDataType;
	using type = std::tuple<
		EnumValuePair<EnumType::BYTE, GL_BYTE>,
		EnumValuePair<EnumType::UNSIGNED_BYTE, GL_UNSIGNED_BYTE>,
		EnumValuePair<EnumType::SHORT, GL_SHORT>,
		EnumValuePair<EnumType::INT, GL_INT>,
		EnumValuePair<EnumType::UNSIGNED_INT, GL_UNSIGNED_INT>,
		EnumValuePair<EnumType::FLOAT, GL_FLOAT>
	>;
};

template<>
struct Moon::Utils::MappingFor<Moon::Rendering::ETextureType, GLenum> {
	using EnumType = Moon::Rendering::ETextureType;
	using type = std::tuple<
		EnumValuePair<EnumType::TEXTURE_2D, GL_TEXTURE_2D>,
		EnumValuePair<EnumType::TEXTURE_3D, GL_TEXTURE_3D>,
		EnumValuePair<EnumType::TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP>
	>;
};