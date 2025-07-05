#pragma once
#include <glad/glad.h>      // for GLshort / GLubyte
#include <cstdint>
inline constexpr const char* fsSrc = R"(
        #version 330 core
        in  vec3 color;
        out vec4 frag_color;
        void main() { frag_color = vec4(color, 1.0); }
    )";

inline constexpr const char* kVertexShader330 = R"(
    #version 330 core

    in ivec2  vertex_position;   
    in uvec3  vertex_color;      

    out vec3 color;
    uniform ivec2 offset;

    void main()
    {
        ivec2 position = vertex_position + offset;
        float x_ndc =  (float(position.x) / 512.0) - 1.0;
        float y_ndc =  1.0 - (float(position.y) / 256.0);
        gl_Position.xyzw = vec4(x_ndc, y_ndc, 0.0, 1.0);

        color = vec3(float(vertex_color.r) / 255.0,
                     float(vertex_color.g) / 255.0,
                      float(vertex_color.b) /255.0);
    }
    )";
    /// 2-D position in PSX coordinates (x, y)
struct Position
{
    GLshort x{};
    GLshort y{};

    /// Parse a GP0 word: 0xYYYYXXXX -> Position(x, y)
    static Position from_gp0(uint32_t val) noexcept
    {
        GLshort x = static_cast<int16_t>(val );
        GLshort y = static_cast<int16_t>(val >> 16);
        return { x, y };
    }
};

/// 24-bit RGB colour (r, g, b)
struct Color
{
    GLubyte r{};
    GLubyte g{};
    GLubyte b{};

    /// Parse a GP0 word: 0x00BBGGRR -> Color(r, g, b)
    static Color from_gp0(uint32_t val) noexcept
    {
        GLubyte r = static_cast<uint8_t>(val );
        GLubyte g = static_cast<uint8_t>(val >> 8 );
        GLubyte b = static_cast<uint8_t>(val >> 16 );
        return { r, g, b };
    }
};

//struct Vertex {
//    std::vector<int16_t> position;
//    std::vector<uint8_t> color;
//    std::vector<uint16_t> texture_coord;
//    static Vertex new_vertex(std::vector<int16_t> pos, std::vector<uint8_t> col) {
//        std::vector<int16_t> position = { pos[0], pos[1] };
//        std::vector<uint8_t> color = { col[0], col[1], col[2] };
//        std::vector<uint16_t> texture_coord = { 0,0 };
//        return { position , color, texture_coord };
//    }
//    static Vertex new_vertex_textured(std::vector<int16_t> pos, std::vector<uint8_t> col, std::vector<uint16_t> texture_c) {
//        std::vector<int16_t> position = { pos[0], pos[1] };
//        std::vector<uint8_t> color = { col[0], col[1], col[2] };
//        std::vector<uint16_t> texture_coord = { texture_c[0],texture_c[1]};
//        return { position , color, texture_coord };
//    }
//
//};