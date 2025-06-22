#pragma once
#include <cstdint>

enum class Field : uint8_t {
    Top = 1,
    Bottom =0,
};

enum class TextureDepth : uint8_t {
    T4Bit = 0,  // 4 bits per pixel
    T8Bit = 1,  // 8 bits per pixel
    T15Bit = 2   // 15 bits per pixel
};

enum class DisplayDepth : uint8_t {
    Depth15Bit = 0,
    Depth24Bit = 1
};


enum class DmaDirection : uint8_t {
    Off = 0,
    Fifo = 1,
    CpuToGp0 = 2,
    VRamToCpu = 3
};
//
// Vertical resolution: 240 or 480 lines
//
enum class VerticalRes : uint8_t {
    Y240 = 0,
    Y480 = 1
};

//
// Video mode: NTSC or PAL
//
enum class VMode : uint8_t {
    NTSC = 0,
    PAL = 1
};

//
// Display color depth: 15-bit or 24-bit output
//
enum class DisplayDepth : uint8_t {
    D15Bits = 0,
    D24Bits = 1
};


//
// Horizontal resolution field (3 bits total: HR1 (2 bits) + HR2 (1 bit))
//
struct HorizontalRes {
    uint8_t value;

    // Construct from HR1 (bits 17-16) and HR2 (bit 18)
    static HorizontalRes from_fields(uint8_t hr1, uint8_t hr2) {
        uint8_t combined = (hr2 & 0x1) | ((hr1 & 0x3) << 1);
        return HorizontalRes{combined};
    }

    // Convert to GPU status register bits [18:16]
    uint32_t into_status() const {
        return static_cast<uint32_t>(value) << 16;
    }
};




struct Gpu {
    // Texture page base coordinates
    uint8_t page_base_x;  // 4 bits (64-byte increment)
    uint8_t page_base_y;  // 1 bit (256-line increment)

    uint8_t semi_transparency;     // Usually 2 bits (blend mode)
    TextureDepth texture_depth;

    bool dithering = false;
    bool draw_to_display = false;
    bool force_set_mask_bit = false;
    bool preserve_masked_pixels = false;

    Field field = Field::Top;
    bool texture_disable = false;

    HorizontalRes hres;
    VerticalRes vres;
    VMode vmode;
    DisplayDepth display_depth;

    bool interlaced = false;
    bool display_disabled = false;

    bool interrupt = false;  // GP0 IRQ pending
    DmaDirection dma_direction = DmaDirection::Off;
};
