#pragma once

#include <Types/Renderer/Texture.h>
#include <map>
#include <mutex>
#include <future>
#include <vector>

namespace Moon::Loaders {
	enum class TextureLoadState { NotLoaded, Decoding, ReadyForUpload, Uploaded, StreamingMips };

	/**
	* Handle the Texture creation and destruction
	*/
	class TextureLoader final {
	public:
		struct Options {
			bool generateMipmaps = true;
			bool sRGB = true;
			bool streamable = true;
			int  maxInFlight = 8;
		};

		/**
		* Disable constructor
		*/
		TextureLoader() = delete;

		static Assets::Texture* Create(
			const std::string& p_filepath,
			const Options& opt = {}
		);

	private:
		static inline std::unordered_map<std::string, Moon::Assets::Texture*> s_textures;
		static inline std::mutex											  s_mutex;

		static void FreeTextureData(Moon::Rendering::Data::TextureData& data);
	};
}