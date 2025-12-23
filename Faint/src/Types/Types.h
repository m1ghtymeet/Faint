#pragma once
#include <Renderer/Types/Vertex2D.h>
#include <Math/Transform.h>
#include <Renderer/Types/Material.h>
#include <bitset>

struct RenderItem {
    uint32_t meshIndex = 0;
    std::shared_ptr<Moon::Rendering::Material> material;
    Moon::Transform transform;
    uint32_t id;
    float distanceToCamera;
};

struct MeshData2D {
    std::vector<Moon::Rendering::Vertex2D> vertices;
    std::vector<uint32_t> indices;
};

/**
* Represents the pipeline state for the scene renderer.
* This structure uses bitfields to minimize memory usage (approximately 16 bytes)
* while allowing fast comparison of states (useful for pipeline state object caching).
* Important note: Bitfield layout order is implementation-defined, but in most modern
*/
struct PipelineState {
	PipelineState()
		: colorWriteMask{ true, true, true, true },
		depthWriteEnable{ true },
		depthTestEnable{ true },
		stencilTestEnable{ false },
		blendingEnable{ false },
		cullingEnable{ true },
		scissorTestEnable{ false },
		multisampleEnable{ true },
		sampleAlphaToCoverageEnable{ false },
		polygonOffsetFillEnable{ false },
		ditherEnable{ false },
		lineWidthPow2{ 0 },                // 2^0 = 1.0
		stencilFuncRef{ 0x00 },
		stencilFuncMask{ 0xFF },
		stencilWriteMask{ 0xFF }
	{}
    union {
        struct {
            // Color Write Mask (4 bits)
            bool colorWriteR : 1;  ///< Enable writing to red channel
            bool colorWriteG : 1;  ///< Enable writing to green channel
            bool colorWriteB : 1;  ///< Enable writing to blue channel
            bool colorWriteA : 1;  ///< Enable writing to alpha channel

            // Boolean Flags (10 bits)
            bool depthWriteEnable : 1;  ///< Enable depth buffer writes
            bool depthTestEnable : 1;  ///< Enable depth testing
            bool stencilTestEnable : 1;  ///< Enable stencil testing
            bool blendingEnable : 1;  ///< Enable alpha blending
            bool cullingEnable : 1;  ///< Enable face culling
            bool scissorTestEnable : 1;  ///< Enable scissor test
            bool multisampleEnable : 1;  ///< Enable multisample anti-aliasing
            bool sampleAlphaToCoverageEnable : 1; ///< Enable sample alpha to coverage
            bool polygonOffsetFillEnable : 1;  ///< Enable polygon offset in fill mode
            bool ditherEnable : 1;  ///< Enable dithering

            // Line Width (3 bits)
            uint8_t lineWidthPow2 : 3;  ///< Line width as power of 2 (0 = 1.0, 1 = 2.0, ...)

            // Padding to align next byte
            uint8_t _padding : 5;  // Optional padding for readability / byte alignment

            // Stencil Parameters
            uint8_t stencilFuncRef : 8;  ///< Reference value for stencil comparison
            uint8_t stencilFuncMask : 8;  ///< Read mask for stencil comparison
            uint8_t stencilWriteMask : 8;  ///< Write mask for stencil buffer

            // You can easily extend this with more fields (blend func, depth func, cull mode, etc.)
        };
        std::bitset<128> _bits;                  ///< Full bit access (useful for equality checks)
        uint8_t          _bytes[16]{};           ///< Raw byte array (ideal for hashing or memcpy)
    };

    struct ColorWriteMask {
        bool r, g, b, a;
    } colorWriteMask;

    // Helper for fast pipeline state comparison (very useful for PSO caching)
    bool operator==(const PipelineState& other) const {
        return _bits == other._bits;
    }

    bool operator!=(const PipelineState& other) const {
        return !(*this == other);
    }
};