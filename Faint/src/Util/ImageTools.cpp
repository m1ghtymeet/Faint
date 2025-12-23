#include "ImageTools.h"
#include "PlatformUtil.h"
#include <Debug/Log.h>
#include <cmp_compressonatorlib/compressonator.h>
#include <fstream>

#include <glad/glad.h>
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

namespace Moon::ImageTools {
    bool g_isCMPFrameworkInitialized = false;

    constexpr int MAX_TEXTURE_SIZE = 1024;

    void LimitTextureSize(Rendering::Data::TextureData& textureData) {
        if (textureData.width <= MAX_TEXTURE_SIZE && textureData.height <= MAX_TEXTURE_SIZE)
            return;

        float aspect = static_cast<float>(textureData.width / textureData.height);

        int newWidth = textureData.width;
        int newHeight = textureData.height;

        if (textureData.width > textureData.height) {
            newWidth = MAX_TEXTURE_SIZE;
            newHeight = static_cast<int>(MAX_TEXTURE_SIZE / aspect);
        }
        else {
            newHeight = MAX_TEXTURE_SIZE;
            newWidth = static_cast<int>(MAX_TEXTURE_SIZE * aspect);
        }

		newWidth = (newWidth + 1) & ~1;
        newHeight = (newHeight + 1) & ~1;

        HZ_CORE_WARN("Texture too large ({}x{}), resizing to {}x{} (max: {})",
            textureData.width, textureData.height, newWidth, newHeight, MAX_TEXTURE_SIZE);

        if (textureData.imageDataType == Rendering::Data::ImageDataType::UNCOMPRESSED && textureData.data) {
            const int channels = textureData.channelCount;
            std::vector<uint8_t> resized(newWidth * newHeight * channels);

            unsigned char* result = stbir_resize_uint8_srgb(
                static_cast<const uint8_t*>(textureData.data), textureData.width, textureData.height, 0,
                resized.data(), newWidth, newHeight, 0,
                static_cast<stbir_pixel_layout>(channels));

            if (!result) {
                HZ_CORE_ERROR("Failed to resize texture using stb_image_resize!");
                return;
            }

            if (textureData.data)
                delete[] static_cast<uint8_t*>(textureData.data);

            textureData.data = new uint8_t[resized.size()];
            std::memcpy(textureData.data, resized.data(), resized.size());

            textureData.width = newWidth;
            textureData.height = newHeight;
            textureData.dataSize = newWidth * newHeight * channels;
        }
    }

    bool CompressionCallback(CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) {
        (pUser1);
        (pUser2);
        std::printf("\rCompression progress = %3.0f", fProgress);
        return false;
    }

    std::string CMPErrorToString(int error) {
        switch (error) {
        case CMP_OK:                            return "Ok.";
        case CMP_ABORTED:                       return "The conversion was aborted.";
        case CMP_ERR_INVALID_SOURCE_TEXTURE:    return "The source texture is invalid.";
        case CMP_ERR_INVALID_DEST_TEXTURE:      return "The destination texture is invalid.";
        case CMP_ERR_UNSUPPORTED_SOURCE_FORMAT: return "The source format is not a supported format.";
        case CMP_ERR_UNSUPPORTED_DEST_FORMAT:   return "The destination format is not a supported format.";
        case CMP_ERR_UNSUPPORTED_GPU_ASTC_DECODE: return "The GPU hardware is not supported for ASTC decoding.";
        case CMP_ERR_UNSUPPORTED_GPU_BASIS_DECODE: return "The GPU hardware is not supported for BASIS decoding.";
        case CMP_ERR_SIZE_MISMATCH:             return "The source and destination texture sizes do not match.";
        case CMP_ERR_UNABLE_TO_INIT_CODEC:      return "Compressonator was unable to initialize the codec needed for conversion.";
        case CMP_ERR_UNABLE_TO_INIT_DECOMPRESSLIB: return "GPU_Decode Lib was unable to initialize the codec needed for decompression.";
        case CMP_ERR_UNABLE_TO_INIT_COMPUTELIB: return "Compute Lib was unable to initialize the codec needed for compression.";
        case CMP_ERR_CMP_DESTINATION:           return "Error in compressing destination texture.";
        case CMP_ERR_MEM_ALLOC_FOR_MIPSET:      return "Memory error: allocating MIPSet compression level data buffer.";
        case CMP_ERR_UNKNOWN_DESTINATION_FORMAT: return "The destination codec type is unknown.";
        case CMP_ERR_FAILED_HOST_SETUP:         return "Failed to setup host for processing.";
        case CMP_ERR_PLUGIN_FILE_NOT_FOUND:     return "The required plugin library was not found.";
        case CMP_ERR_UNABLE_TO_LOAD_FILE:       return "The requested file was not loaded.";
        case CMP_ERR_UNABLE_TO_CREATE_ENCODER:  return "Request to create an encoder failed.";
        case CMP_ERR_UNABLE_TO_LOAD_ENCODER:    return "Unable to load an encoder library.";
        case CMP_ERR_NOSHADER_CODE_DEFINED:     return "No shader code is available for the requested framework.";
        case CMP_ERR_GPU_DOESNOT_SUPPORT_COMPUTE: return "The GPU device selected does not support compute.";
        case CMP_ERR_NOPERFSTATS:               return "No performance stats are available.";
        case CMP_ERR_GPU_DOESNOT_SUPPORT_CMP_EXT: return "The GPU does not support the requested compression extension.";
        case CMP_ERR_GAMMA_OUTOFRANGE:          return "Gamma value set for processing is out of range.";
        case CMP_ERR_PLUGIN_SHAREDIO_NOT_SET:   return "The plugin shared IO call was not set and is required for this plugin to operate.";
        case CMP_ERR_UNABLE_TO_INIT_D3DX:       return "Unable to initialize DirectX SDK or get a specific DX API.";
        case CMP_FRAMEWORK_NOT_INITIALIZED:     return "CMP_InitFramework failed or not called.";
        case CMP_ERR_GENERIC:                   return "An unknown error occurred.";
        default:                                return "Unknown CMP_ERROR value.";
        }
    }
}

Moon::ImageTools::DDSFormatInfo Moon::ImageTools::GetDDSFormatInfo(const DDSHeader& header, DDSHeaderDX10* dx10Header) {
    DDSFormatInfo formatInfo = {};
    if (header.ddspf_dwFourCC == FOURCC_DXT1) {
        formatInfo.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        formatInfo.format = GL_RGBA; // DXT1 represents RGB data with optional alpha
        formatInfo.blockSize = 8;
        formatInfo.channelCount = (header.ddspf_dwABitMask) ? 4 : 3; // DXT1 may omit alpha
    }
    else if (header.ddspf_dwFourCC == FOURCC_DXT3 || header.ddspf_dwFourCC == FOURCC_DXT5) {
        formatInfo.internalFormat = (header.ddspf_dwFourCC == FOURCC_DXT3)
            ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
            : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        formatInfo.format = GL_RGBA; // DXT3/5 represent RGBA data
        formatInfo.blockSize = 16;
        formatInfo.channelCount = 4; // DXT3/5 store RGBA
    }
    else if (header.ddspf_dwFourCC == FOURCC_DX10 && dx10Header) {
        // DX10 header is present, check dxgiFormat
        switch (dx10Header->dxgiFormat) {
        case 98: // DXGI_FORMAT_BC7_UNORM
            formatInfo.internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
            formatInfo.format = GL_RGBA; // BC7 represents RGBA data
            formatInfo.blockSize = 16;
            formatInfo.channelCount = 4; // BC7 is always RGBA
            break;
        case 99: // DXGI_FORMAT_BC7_UNORM_SRGB
            formatInfo.internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
            formatInfo.format = GL_SRGB_ALPHA; // BC7 in sRGB
            formatInfo.blockSize = 16;
            formatInfo.channelCount = 4; // BC7 is always RGBA
            break;
        default:
            HZ_CORE_ERROR("Unsupported DX10 format : " + std::to_string(dx10Header->dxgiFormat));
            return {};
        }
    }
    else {
        HZ_CORE_ERROR("Unsupported DDS format: " + std::to_string(header.ddspf_dwFourCC));
        return {};
    }
    return formatInfo;
}

void Moon::ImageTools::InitializeCMPFramework() {
    CMP_InitFramework();
    g_isCMPFrameworkInitialized = true;
}

void Moon::ImageTools::CreateAndExportDDS(const std::string& inputFilepath, const std::string& outputFilepath, bool generateMipMaps) {
    if (!g_isCMPFrameworkInitialized) {
        InitializeCMPFramework();
    }

    CMP_MipSet mipSetIn = {};
    CMP_MipSet mipSetOut = {};
    //CMP_MipSet mipSetResized = {};
    KernelOptions kernelOptions = {};
    CMP_ERROR status;

    // Load the texture
    status = CMP_LoadTexture(inputFilepath.c_str(), &mipSetIn);
    if (status != CMP_OK) {
        HZ_CORE_ERROR("Error: Failed to load texture. Error code: " + CMPErrorToString(status));
        return;
    }
    //memset(&mipSetResized, 0, sizeof(CMP_MipSet));
    

    // Generate mipmaps
    //if (generateMipMaps) {
    //    CMP_INT mipmapLevelCount = (CMP_INT)(std::log2(std::max(mipSetIn.m_nWidth, mipSetIn.m_nHeight))) + 1;
    //    CMP_INT minSize = CMP_CalcMinMipSize(mipSetIn.m_nHeight, mipSetIn.m_nWidth, mipmapLevelCount);
    //    CMP_GenerateMIPLevels(&mipSetIn, minSize);
    //}
    // Compression settings
    kernelOptions.encodeWith = CMP_CPU; // CMP_HPC; // CMP_GPU_OCL
    kernelOptions.format = CMP_FORMAT_BC1;
    kernelOptions.fquality = 0.88;
    kernelOptions.threads = 0;

    memset(&mipSetOut, 0, sizeof(CMP_MipSet));
    status = CMP_ProcessTexture(&mipSetIn, &mipSetOut, kernelOptions, CompressionCallback);
    if (status != CMP_OK) {
        HZ_CORE_ERROR("Failed to process texture " + inputFilepath + ": " + CMPErrorToString(status));
        return;
    }
    status = CMP_SaveTexture(outputFilepath.c_str(), &mipSetOut);
    if (status != CMP_OK) {
        CMP_FreeMipSet(&mipSetIn);
        HZ_CORE_ERROR("Failed to save texture " + inputFilepath + ": " + CMPErrorToString(status));
        return;
    }
    // Cleanup
    CMP_FreeMipSet(&mipSetIn);
    CMP_FreeMipSet(&mipSetOut);
}

Moon::Rendering::Data::TextureData Moon::ImageTools::LoadUncompressedTextureData(const std::string& filepath) {
    stbi_set_flip_vertically_on_load(true);

    Rendering::Data::TextureData textureData;
    uint8_t* imageData = stbi_load(filepath.data(), &textureData.width, &textureData.height, &textureData.channelCount, 0);
    textureData.imageDataType = Rendering::Data::ImageDataType::UNCOMPRESSED;
    
    if (textureData.channelCount == 3) {
        size_t newSize = textureData.width * textureData.height * 4;
        uint8_t* rgbaData = new uint8_t[newSize];
        for (size_t i = 0, j = 0; i < newSize; i += 4, j += 3) {
            rgbaData[i] = imageData[j];         // R
            rgbaData[i + 1] = imageData[j + 1]; // G
            rgbaData[i + 2] = imageData[j + 2]; // B
            rgbaData[i + 3] = 255;              // A
        }
        stbi_image_free(imageData);
        textureData.data = rgbaData;
        textureData.channelCount = 4;
    }
    else {
        textureData.data = imageData;
    }

    if (textureData.channelCount == 4 && textureData.width != textureData.height) {
        textureData.dataSize = textureData.width * textureData.height * 4;
    }
    else {
        textureData.dataSize = textureData.width * textureData.height * textureData.channelCount;
    }

    textureData.format = GL_RGBA;
    textureData.internalFormat = GL_RGBA8; // Use GL_RGBA8 instead of GL_RGBA16F for less memory

    LimitTextureSize(textureData);

    return textureData;
}

Moon::Rendering::Data::TextureData Moon::ImageTools::LoadCompressedTextureDataFromDDS(const std::string& filepath, bool flipVertically) {
    Rendering::Data::TextureData textureData;
    textureData.imageDataType = Rendering::Data::ImageDataType::COMPRESSED;
    
    // Open the file in binary mode
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        HZ_CORE_ERROR("Failed to open DDS file: " + filepath);
        return textureData;
    }
    // Read the validate the DDS header
    DDSHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.dwMagic != 0x20534444) {
        HZ_CORE_ERROR("Not a valid DDS file: " + filepath);
        return textureData;
    }
    // Check for potential DX10 extended header
    DDSHeaderDX10 dx10Header = {};
    if (header.ddspf_dwFourCC == 0x30315844) { // "DX10" FourCC
        file.read(reinterpret_cast<char*>(&dx10Header), sizeof(dx10Header));
    }
    // Retrieve format information
    DDSFormatInfo formatInfo = GetDDSFormatInfo(header, &dx10Header);
    
    uint32_t blockWide = (header.dwWidth + 3) / 4;
    uint32_t blockHeigh = (header.dwHeight + 3) / 4;
    uint32_t dataSize = blockWide * blockHeigh * formatInfo.blockSize;
    
    std::vector<char> buffer(dataSize);
    file.read(buffer.data(), dataSize);
    if (file.gcount() != static_cast<std::streamsize>(dataSize)) {
        HZ_CORE_ERROR("Failed to read texture data: " + filepath);
        return textureData;
    }
    
    //if (flipVertically) {
    //    int stride = blockWide * formatInfo.blockSize;
    //    std::vector<char> temp(stride);
    //    for (uint32_t y = 0; y < blockHeigh / 2; y++) {
    //        char* rowTop = buffer.data() + y * stride;
    //        char* rowBottom = buffer.data() + (blockHeigh - 1 - y) * stride;
    //        memcpy(temp.data(), rowTop, stride);
    //        memcpy(rowTop, rowBottom, stride);
    //        memcpy(rowBottom, temp.data(), stride);
    //    }
    //}
    
    textureData.dataSize = dataSize;
    textureData.data = new char[dataSize];
    std::memcpy(textureData.data, buffer.data(), dataSize);
    textureData.width = header.dwWidth;
    textureData.height = header.dwHeight;
    textureData.internalFormat = formatInfo.internalFormat;
    textureData.format = formatInfo.format;
    textureData.channelCount = formatInfo.channelCount;
    
    file.close();
    return textureData;
}

std::vector<Moon::Rendering::Data::TextureData> Moon::ImageTools::LoadTextureDataFromDDS(const std::string& filepath) {
    std::vector<Rendering::Data::TextureData> textureDataLevels;

    // Open the file in binary mode
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        HZ_CORE_ERROR("Failed to open DDS file: " + filepath);
        return textureDataLevels;
    }
    // Read the validate the DDS header
    DDSHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.dwMagic != 0x20534444) {
        HZ_CORE_ERROR("Not a valid DDS file: " + filepath);
        return textureDataLevels;
    }
    // Check for potential DX10 extended header
    DDSHeaderDX10 dx10Header = {};
    if (header.ddspf_dwFourCC == 0x30315844) { // "DX10" FourCC
        file.read(reinterpret_cast<char*>(&dx10Header), sizeof(dx10Header));
    }
    // Retrieve format information
    DDSFormatInfo formatInfo = GetDDSFormatInfo(header, &dx10Header);

    // Iterate the mipmap levels
    uint32_t originalWidth = header.dwWidth;
    uint32_t originalHeight = header.dwHeight;
    const bool tooLarge = originalWidth > MAX_TEXTURE_SIZE || originalHeight > MAX_TEXTURE_SIZE;
    if (tooLarge)
        HZ_CORE_WARN("DDS texture {}x{} exceeds {} limit. Only base level will be loaded. Mimaps will be regenerated on GPU.",
            originalWidth, originalHeight, MAX_TEXTURE_SIZE);

    uint32_t mipWidth = header.dwWidth;
    uint32_t mipHeight = header.dwHeight;
    uint32_t mipCount = tooLarge ? 1 : std::max(1u, header.dwMipMapCount);

    for (uint32_t i = 0; i < mipCount; i++) {
        uint32_t blocksWide = (mipWidth + 3) / 4;
        uint32_t blocksHeigh = (mipHeight + 3) / 4;
        uint32_t dataSize = blocksWide * blocksHeigh * formatInfo.blockSize;

        // Read the mipmap data
        std::vector<char> buffer(dataSize);
        file.read(buffer.data(), dataSize);
        if (file.gcount() != static_cast<std::streamsize>(dataSize)) {
            HZ_CORE_ERROR("Reading mip level " + std::to_string(i));
            break;
        }
        // Store the mipmap data
        Rendering::Data::TextureData& textureData = textureDataLevels.emplace_back();
        textureData.dataSize = dataSize;
        textureData.data = new char[dataSize];
        std::memcpy(textureData.data, buffer.data(), dataSize);
        textureData.width = mipWidth;
        textureData.height = mipHeight;
        textureData.internalFormat = formatInfo.internalFormat;
        textureData.format = formatInfo.format;
        textureData.channelCount = formatInfo.channelCount;
        mipWidth = std::max(1u, mipWidth / 2);
        mipHeight = std::max(1u, mipHeight / 2);
    }

    file.close();
    return textureDataLevels;
}