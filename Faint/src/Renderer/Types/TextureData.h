#pragma once

#include <optional>
#include <Renderer/Enums/Format.h>
#include <Renderer/Enums/PixelDataType.h>

namespace Moon::Rendering::Data {

	enum class ImageDataType {
		UNCOMPRESSED,
		COMPRESSED
	};

	struct TextureData {
		int width = 0;
		int height = 0;
		int channelCount = 0;
		int dataSize = 0;
		int format = 0;
		int internalFormat = 0;
		void* data = nullptr;
		bool useMipMaps = true;
		ImageDataType imageDataType;
	};
}