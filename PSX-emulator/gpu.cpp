#include "gpu.hpp"
#include <stdio.h>
#include <stdexcept>


void GPU::reset() {
	page_base_x = 0;
	page_base_y = 0;
	semi_transparency = 0;
	texture_depth = TextureDepth::T4Bit;
	dithering = false;
	draw_to_display = false;
	force_set_mask_bit = false;
	preserve_masked_pixels = false;
	field = Field::Top;
	texture_disable = false;
	hres = HorizontalRes::from_fields(0, 0);
	vres = VerticalRes::Y240;
	vmode = VMode::NTSC;
	display_depth = DisplayDepth::Depth15Bit;
	interlaced = false;
	display_disabled = true;
	interrupt = false;
	dma_direction = DmaDirection::Off;
    gp0_mode = Gp0Mode::Command;
    gp0_command_remaining = 0;
    gp0_command_method = nullptr;
    gp1_reset(0);

}

uint32_t GPU::status()  {
    uint32_t r = 0;

    r |= static_cast<uint32_t>(page_base_x) << 0;
    r |= static_cast<uint32_t>(page_base_y) << 4;
    r |= static_cast<uint32_t>(semi_transparency) << 5;
    r |= static_cast<uint32_t>(texture_depth) << 7;
    r |= static_cast<uint32_t>(dithering) << 9;
    r |= static_cast<uint32_t>(draw_to_display) << 10;
    r |= static_cast<uint32_t>(force_set_mask_bit) << 11;
    r |= static_cast<uint32_t>(preserve_masked_pixels) << 12;
    r |= static_cast<uint32_t>(field) << 13;
    r |= static_cast<uint32_t>(texture_disable) << 15;

    r |= hres.into_status();  // handles bits 18–16
   

    //r |= static_cast<uint32_t>(vres) << 19;
    r |= static_cast<uint32_t>(vmode) << 20;
    r |= static_cast<uint32_t>(display_depth) << 21;
    r |= static_cast<uint32_t>(interlaced) << 22;
    r |= static_cast<uint32_t>(display_disabled) << 23;
    r |= static_cast<uint32_t>(interrupt) << 24;

    // Status bits for readiness (always ready for now)
    r |= 1 << 26;  // Ready to receive GP0
    r |= 1 << 27;  // Ready to send VRAM to CPU
    r |= 1 << 28;  // Ready for DMA block transfer

    r |= static_cast<uint32_t>(dma_direction) << 29;

    // Bit 31: usually 0 unless vertical blank/even line/etc. Ignored here
    r |= 0 << 31;

    // DMA request signal for "DMA in" on request mode
    uint32_t dma_request = 0;
    switch (dma_direction) {
    case DmaDirection::Off:
        dma_request = 0;
        break;
    case DmaDirection::CpuToGp0:
        dma_request = (r >> 28) & 1;  // same as DMA ready
        break;
    case DmaDirection::VRamToCpu:
        dma_request = (r >> 27) & 1;  // same as VRAM ready
        break;
    case DmaDirection::Fifo:
        dma_request = 1;  // always ready if not full (assumed)
        break;
    }

    r |= (dma_request << 25);

    return r;
}

void GPU::gp0(uint32_t val) {
	
    if (gp0_command_remaining == 0) {

        uint8_t opcode = (val >> 24) & 0xFF;
       // printf("\nGP0: 0x%08X", val);
        switch (opcode) {
        case 0x00:
            gp0_command_remaining = 1;
            gp0_command_method = &GPU::gp0_nop;
            break;
       /* case 0x38:
            gp0_command_remaining = 8;
            gp0_command_method = &GPU::gp0_quad_shaded_opaque;
            break;*/
        case 0x30:
            gp0_command_remaining = 6;
            gp0_command_method = &GPU::gp0_triangle_shaded_opaque;
            break;
        case 0x2c:
            gp0_command_remaining = 9;
            gp0_command_method = &GPU::gp0_quad_texture_blend_opaque;
            break;
        case 0xa0:
            gp0_command_remaining = 3;
            gp0_command_method = &GPU::gp0_image_load;
            break;
        case 0x28:
            gp0_command_remaining = 5;
            gp0_command_method = &GPU::gp0_quad_mono_opaque;
            break;
        case 0xE1:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) { GPU::gp0_draw_mode(gpu,val); };
            
            break;
        case 0xE2:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) {GPU::gp0_texture_window(gpu, val);};
            break;
        case 0xE3:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) {GPU::gp0_drawing_area_top_left(gpu, val);};
            break;
        case 0xE4:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) {GPU::gp0_drawing_area_bottom_right(gpu, val);};
            break;
        case 0xE5:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) {GPU::gp0_drawing_offset(gpu, val);};
            break;
        case 0xE6:
            gp0_command_remaining = 1;
            gp0_command_method = [val](GPU* gpu) {GPU::gp0_mask_bit_setting(gpu, val);};
            break;
        case 0x01:
			gp0_command_remaining = 1;
			gp0_command_method = &GPU::gp0_clear_cache;
            break;
        case 0xc0:
            gp0_command_remaining = 3;
            gp0_command_method = &GPU::gp0_image_store;
			break;
        case 0x38:
            gp0_command_remaining = 8;
            gp0_command_method = &GPU::gp0_quad_shaded_opaque;
			break;

        default:
            gp0_command_remaining = 1;
            printf("\nUnhandled GP0 command: 0x%08x",val);
            printf("\nI do not handle this helppppppppppppppppppppppppppppppppp\n");
            //throw std::exception("help");
            break;
        }
        gp0_command.clear();
    }
    
    gp0_command_remaining -= 1;
    switch (gp0_mode) {
    case Gp0Mode::Command: {
        gp0_command.push_word(val);
        if (gp0_command_remaining == 0) {
            (gp0_command_method)(this);
        }
        
    }
    case Gp0Mode::ImageLoad: {
        if (gp0_command_remaining == 0) {
            gp0_mode = Gp0Mode::Command;
        }
    }
    }
}

void GPU::gp1(uint32_t val) {
   // printf("\nGP1: 0x%08X", val);
    auto opcode = (val >> 24) & 0xFF;
    switch (opcode) {
    case 0x0: gp1_reset(0x0);break;
    case 0x01: gp1_reset_command_buffer();break;
    case 0x02: gp1_acknowledge_irq();break;
    case 0x03: gp1_display_enable(val); break;
    case 0x04: gp1_dma_direction(val); break;
    case 0x05: gp1_display_vram_start(val); break;
    case 0x06: gp1_display_horizontal_range(val); break;
    case 0x07: gp1_display_vertical_range(val); break;
    case 0x08: gp1_display_mode(val); break;
	
    default:
        printf("\nI do not handle this helppppppppppppppppppppppppppppppppp\n");
        break;
    }
}
void GPU::gp0_image_load(GPU* self) {
    // command words were already pushed: [A0xx.., XY, WH]
    uint16_t width = (self->gp0_command[2] & 0xFFFF) ;
    uint16_t height = ((self->gp0_command[2] >> 16) ) ;
	uint32_t imgsize = ((width * height) + 1) & ~1; // Ensure even size for 16-bit pixels

    self->gp0_command_remaining = imgsize / 2;   // 16-bit pixels

    self->gp0_mode = Gp0Mode::ImageLoad;

}

void GPU::gp0_nop(GPU* self) {
    // NOP OPCODE
}
void GPU::gp0_quad_mono_opaque(GPU* self) {
    std::vector<Position> positions = { Position::from_gp0(self->gp0_command[1]),
                                        Position::from_gp0(self->gp0_command[2]),
                                        Position::from_gp0(self->gp0_command[3]),
                                        Position::from_gp0(self->gp0_command[4])
    };
    std::vector<Color> colors = {   Color::from_gp0(self->gp0_command[0]),
                                    Color::from_gp0(self->gp0_command[0]),
                                    Color::from_gp0(self->gp0_command[0]),
                                    Color::from_gp0(self->gp0_command[0])
    };
    self->renderer.push_quad(&positions, &colors);
}
void GPU::gp0_draw_mode(GPU* self, uint32_t val) {
    self->page_base_x = static_cast<uint8_t>(val & 0xF);
    self->page_base_y = static_cast<uint8_t>((val >> 4) & 0x1);

    self->semi_transparency = static_cast<uint8_t>((val >> 5) & 3);

    switch ((val >> 7) & 3) {
    case 0: self->texture_depth = TextureDepth::T4Bit; break;
    case 1: self->texture_depth = TextureDepth::T8Bit; break;
    case 2: self->texture_depth = TextureDepth::T15Bit; break;
    default: printf("Unhandled texture depth\n"); break;
    }

    self->dithering = ((val >> 9) & 1) != 0;
    self->draw_to_display = ((val >> 10) & 1) != 0;
    self->texture_disable = ((val >> 11) & 1) != 0;
    self->rectangle_texture_x_flip = ((val >> 12) & 1) != 0;
    self->rectangle_texture_y_flip = ((val >> 13) & 1) != 0;
}


void GPU::gp1_reset(uint32_t val) {
    gp1_reset_command_buffer();
    gp1_acknowledge_irq();
    interrupt = false;

    page_base_x = 0;
    page_base_y = 0;
    semi_transparency = 0;
    texture_depth = TextureDepth::T4Bit;

    texture_window_x_mask = 0;
    texture_window_y_mask = 0;
    texture_window_x_offset = 0;
    texture_window_y_offset = 0;

    dithering = false;
    draw_to_display = false;
    texture_disable = false;

    rectangle_texture_x_flip = false;
    rectangle_texture_y_flip = false;

    drawing_area_left = 0;
    drawing_area_top = 0;
    drawing_area_right = 0;
    drawing_area_bottom = 0;

    drawing_x_offset = 0;
    drawing_y_offset = 0;

    force_set_mask_bit = false;
    preserve_masked_pixels = false;

    dma_direction = DmaDirection::Off;
    display_disabled = true;

    display_vram_x_start = 0;
    display_vram_y_start = 0;

    hres = HorizontalRes::from_fields(0, 0);
    vres = VerticalRes::Y240;
    vmode = VMode::NTSC;
    interlaced = true;

    display_horiz_start = 0x200;
    display_horiz_end = 0xC00;
    display_line_start = 0x10;
    display_line_end = 0x100;

    display_depth = DisplayDepth::Depth15Bit;
}

void GPU::gp1_display_mode(uint32_t val) {
    uint8_t hr1 = static_cast<uint8_t>(val & 0x3);
    uint8_t hr2 = static_cast<uint8_t>((val >> 6) & 0x1);

    hres = HorizontalRes::from_fields(hr1, hr2);

    vres = ((val & 0x4) !=0) ? VerticalRes::Y480 : VerticalRes::Y240;
    vmode = ((val & 0x8) !=0) ? VMode::PAL : VMode::NTSC;
    display_depth = ((val & 0x10) !=0) ? DisplayDepth::Depth24Bit : DisplayDepth::Depth15Bit;
    interlaced = (val & 0x20) != 0;

    if ((val & 0x80) !=0 ) {
        throw std::runtime_error("Unsupported display mode: ");
    }
}

void GPU::gp1_dma_direction(uint32_t val) {
    switch (val & 0x3) {
    case 0: dma_direction = DmaDirection::Off; break;
    case 1: dma_direction = DmaDirection::Fifo; break;
    case 2: dma_direction = DmaDirection::CpuToGp0; break;
    case 3: dma_direction = DmaDirection::VRamToCpu; break;
    default:
        throw std::runtime_error("Unreachable: invalid DMA direction");
    }
}

void GPU::gp0_drawing_area_top_left(GPU* self, uint32_t val){
    self->drawing_area_top = static_cast<uint16_t>((val >> 10) & 0x3FF);
    self->drawing_area_left = static_cast<uint16_t>(val & 0x3FF);
}

void GPU::gp0_drawing_area_bottom_right(GPU* self, uint32_t val) {
    self->drawing_area_bottom = static_cast<uint16_t>((val >> 10) & 0x3FF);
    self->drawing_area_right = static_cast<uint16_t>(val  & 0x3FF);
}

void GPU::gp0_drawing_offset(GPU* self, uint32_t val) {
    auto x = static_cast<uint16_t>(val & 0x7FF);
    auto y = static_cast<uint16_t>((val >> 11) & 0x7FF);

    self->drawing_x_offset = (static_cast<int16_t>(x << 5)) >> 5;
    self->drawing_y_offset = (static_cast<int16_t>(y << 5)) >> 5;
    self->renderer.set_draw_offset(x, y);
    self->renderer.display();
}

void GPU::gp0_texture_window(GPU* self,uint32_t val) {
    self->texture_window_x_mask = static_cast<uint8_t>(val & 0x1F);
    self->texture_window_y_mask = static_cast<uint8_t>((val >>5) & 0x1F);
    self->texture_window_x_offset = static_cast<uint8_t>((val >> 10) & 0x1F);
    self->texture_window_y_offset = static_cast<uint8_t>((val >> 15) & 0x1F);
}

void GPU::gp0_mask_bit_setting(GPU* self,uint32_t val) {
    self->force_set_mask_bit = (val & 1) != 0;
    self->preserve_masked_pixels = (val & 2) != 0;
}

void GPU::gp1_display_enable(uint32_t val) {
    display_disabled = ((val & 1) != 0);
}


void GPU::update_viewport()
{
    /* 1. calculate width/height ----------------------------------- */
    const uint16_t width =
        static_cast<uint16_t>((display_horiz_end - display_horiz_start) );
    const uint16_t height =
        static_cast<uint16_t>(display_line_end - display_line_start);

    if (width == 0 || width > 1024 || height == 0 || height > 512)
        return;                               // BIOS still initialising

    /* 2. origin inside VRAM -------------------------------------- */
    const uint16_t x_off = static_cast<uint16_t>((display_horiz_start - 0x200) );
    const uint16_t y_off = display_line_start;

    const uint16_t gl_x = static_cast<uint16_t>(display_vram_x_start + x_off);
    const uint16_t gl_y = static_cast<uint16_t>(display_vram_y_start + y_off);
    // ^ NO extra flip – shader already does it

    renderer.set_display_window(gl_x, gl_y, width, height);
}


void GPU::gp1_display_vram_start(uint32_t val) {
    display_vram_x_start = static_cast<uint16_t>(val & 0x3FE);
    display_vram_y_start = static_cast<uint16_t>((val >> 10) & 0x1FF);
    update_viewport();
}
void GPU::gp1_display_horizontal_range(uint32_t val) {
    display_horiz_start = static_cast<uint16_t>(val & 0xFFF);
    display_horiz_end = static_cast<uint16_t>((val >> 12) & 0xFFF); 
    update_viewport();
}
void GPU::gp1_display_vertical_range(uint32_t val) {
    display_line_start = static_cast<uint16_t>(val & 0x3FF);
    display_line_end = static_cast<uint16_t>((val >> 10) & 0x3FF);
    update_viewport();
}

void GPU::gp0_image_store() {
    // Parameter 2 contains the image resolution
    auto res = gp0_command[2];
    auto width = res & 0xffff;
    auto height = res >> 16;
    //printf("unhandled image store");
}

void GPU::gp1_reset_command_buffer() {
    gp0_command.clear();
    gp0_command_remaining = 0;
    gp0_mode = Gp0Mode::Command;
}
void GPU::gp0_quad_shaded_opaque(GPU* self) {
    std::vector<Position> positions = { Position::from_gp0(self->gp0_command[1]),
                                        Position::from_gp0(self->gp0_command[3]),
                                        Position::from_gp0(self->gp0_command[5]),
                                        Position::from_gp0(self->gp0_command[7])
    };
    std::vector<Color> colors = { Color::from_gp0(self->gp0_command[0]),
                                    Color::from_gp0(self->gp0_command[2]),
                                    Color::from_gp0(self->gp0_command[4]),
                                    Color::from_gp0(self->gp0_command[6])
    };
    self->renderer.push_quad(&positions, &colors);
}
// GP0(0x30) - Triangle shaded opaque
void GPU::gp0_triangle_shaded_opaque(GPU* self) {
    std::vector<Position> positions = { Position::from_gp0(self->gp0_command[1]),
                       Position::from_gp0(self->gp0_command[3]),
        Position::from_gp0(self->gp0_command[5]) };
    std::vector<Color> colors = { Color::from_gp0(self->gp0_command[0]),
                       Color::from_gp0(self->gp0_command[2]),
		Color::from_gp0(self->gp0_command[4]) };
	self ->renderer.push_triangle(&positions, &colors);
    //printf("Unhandled triangle");
}
void GPU::gp0_quad_texture_blend_opaque(GPU* self) {
    std::vector<Position> positions = { Position::from_gp0(self->gp0_command[1]),
                                        Position::from_gp0(self->gp0_command[3]),
                                        Position::from_gp0(self->gp0_command[5]),
                                        Position::from_gp0(self->gp0_command[7])
    };
    std::vector<Color> colors = {Color(0x80,0x00,0x00),
                                Color(0x80,0x00,0x00),
                                Color(0x80,0x00,0x00),
                                Color(0x80,0x00,0x00)
    };
    self->renderer.push_quad(&positions, &colors);
}
void GPU::gp1_acknowledge_irq() {
    interrupt = false;
}

uint32_t GPU::read() {
    return 0;
}
std::vector<uint16_t> GPU::gp0_texture_coordinates(uint32_t gp0) {
    auto x = gp0 & 0xFF;
    auto y = (gp0 >> 8) & 0xFF;
    return std::vector<uint16_t>{static_cast<uint16_t>(x), static_cast<uint16_t>(y)};
}
