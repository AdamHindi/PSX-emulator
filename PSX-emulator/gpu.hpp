#pragma once
#include <cstdint>
#include <functional>

enum class Gp0Mode {
    /// D e f a ul t mode : h a n dli n g commands
    Command,
    /// Loading an image i n t o VRAM
    ImageLoad,
};

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


struct CommandBuffer {
    uint32_t buffer[15];
    uint8_t len;
    CommandBuffer() : len(0) {
        std::fill(std::begin(buffer), std::end(buffer), 0);
	}

    void clear() { len = 0; };
    void push_word(uint32_t word) {

        buffer[len] = word;
        len += 1;
        
    };

    void reset() {
        len = 0;
        std::fill(std::begin(buffer), std::end(buffer), 0);
    }
     uint32_t operator[](size_t index) {
        if (index >= static_cast<size_t>(len)) {
            throw std::exception("Command buffer index out of range: ");
        }
        return buffer[index];
    }
};

struct GPU {
    Gp0Mode gp0_mode;

    CommandBuffer gp0_command;
    uint32_t gp0_command_remaining;
    std::function<void(GPU*)> gp0_command_method;


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
    
    bool rectangle_texture_x_flip;
    bool rectangle_texture_y_flip;

    // Texture window masking and offset (each in 8-pixel steps)
    uint8_t texture_window_x_mask;    // GP1(0x05)
    uint8_t texture_window_y_mask;
    uint8_t texture_window_x_offset;
    uint8_t texture_window_y_offset;

    // Drawing area bounds (in framebuffer coordinates)
    uint16_t drawing_area_left;       // GP0(0xE3)
    uint16_t drawing_area_top;        // GP0(0xE3)
    uint16_t drawing_area_right;      // GP0(0xE4)
    uint16_t drawing_area_bottom;     // GP0(0xE4)

    // Drawing offset (signed offsets applied to all vertices)
    int16_t drawing_x_offset;         // GP0(0xE5)
    int16_t drawing_y_offset;

    // Display area in VRAM (where pixels are copied from for output)
    uint16_t display_vram_x_start;    // GP1(0x06)
    uint16_t display_vram_y_start;

    // Display timing (relative to HSYNC/VSYNC)
    uint16_t display_horiz_start;     // GP1(0x07)
    uint16_t display_horiz_end;
    uint16_t display_line_start;
    uint16_t display_line_end;

    // Functions
    void reset();
    uint32_t read();
    void gp0_clear_cache() {};
    void gp1_reset(uint32_t val);

    GPU() {
        reset();

    }
    uint32_t status();
    void gp0(uint32_t val);
    void gp1(uint32_t val);
    void gp1_display_mode(uint32_t val);
    void gp1_dma_direction(uint32_t val);
    void gp1_display_enable(uint32_t val);
    void gp1_display_vram_start(uint32_t val);
	void gp1_display_horizontal_range(uint32_t val);
	void gp1_display_vertical_range(uint32_t val);
    
    void gp0_image_store();
    void gp1_acknowledge_irq();
    void gp1_reset_command_buffer();
    
    static void gp0_quad_shaded_opaque(GPU* self);
    static void gp0_triangle_shaded_opaque(GPU* self);
    static void gp0_quad_texture_blend_opaque(GPU* self);
    

    static void gp0_image_load(GPU* self);
    static void gp0_nop(GPU* self);
    static void gp0_quad_mono_opaque(GPU* self);
    static void gp0_draw_mode(GPU* self, uint32_t val);
    static void gp0_drawing_area_top_left(GPU* self, uint32_t val);
    static void gp0_drawing_area_bottom_right(GPU* self, uint32_t val);
    static void gp0_drawing_offset(GPU* self, uint32_t val);
    static void gp0_texture_window(GPU* self, uint32_t val);
    static void gp0_mask_bit_setting(GPU* self, uint32_t val);
};
