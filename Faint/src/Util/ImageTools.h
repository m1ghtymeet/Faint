#pragma once

#include <Renderer/Types/TextureData.h>
#include <string>
#include <vector>

#define FOURCC_DXT1 0x31545844 // "DXT1"
#define FOURCC_DXT3 0x33545844 // "DXT3"
#define FOURCC_DXT5 0x35545844 // "DXT5"
#define FOURCC_DX10 0x30315844 // "DX10"

/* DDS */
struct DDSHeader {
    uint32_t dwMagic;
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    uint32_t ddspf_dwSize;
    uint32_t ddspf_dwFlags;
    uint32_t ddspf_dwFourCC;
    uint32_t ddspf_dwRGBBitCount;
    uint32_t ddspf_dwRBitMask;
    uint32_t ddspf_dwGBitMask;
    uint32_t ddspf_dwBBitMask;
    uint32_t ddspf_dwABitMask;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

struct DDSHeaderDX10 {
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t reserved;
};

namespace Moon::ImageTools {

    struct DDSFormatInfo {
        int internalFormat; // GPU internal storage format
        int format;         // Data format (if applicable, e.g., GL_RGBA for uncompressed)
        int blockSize;      // Block size for compressed formats
        int channelCount;   // Number of channels (1 for grayscale, 3 for RGB, 4 for RGBA)
    };

    DDSFormatInfo GetDDSFormatInfo(const DDSHeader& header, DDSHeaderDX10* dx10Header);

    void InitializeCMPFramework();
    void CreateAndExportDDS(const std::string& inputFilepath, const std::string& outputFilepath, bool generateMipMaps);

    Rendering::Data::TextureData LoadUncompressedTextureData(const std::string& filepath);
    Rendering::Data::TextureData LoadCompressedTextureDataFromDDS(const std::string& filepath, bool flipVertically = true);
	std::vector<Rendering::Data::TextureData> LoadTextureDataFromDDS(const std::string& filepath);
	Rendering::Data::TextureData LoadR16FTextureData(const std::string& filepath);
}