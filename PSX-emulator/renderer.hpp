#pragma once
#include "buffer.hpp"
#include "render_helpers.hpp"
#include <SDL.h>  
#include <cstring> // Include for std::memcpy  
#include <optional>
#include "debug.hpp"
#include <cstdint>
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

    // call once per frame
    void render();
    void push_triangle(std::vector<Position>* positions, std::vector<Color>* colors);
    void push_quad(std::vector<Position>* positions, std::vector<Color>* colors);
    void draw();
    void display();
    void set_draw_offset(int16_t x, int16_t y);


    SDL_Window* window_ = nullptr;
    SDL_GLContext   glCtx_ = nullptr;
    GLuint program;
    GLuint vertexShader;
	GLuint fragmentShader;
    GLuint shaderProgram;
    GLuint vertexArrayObject;
    Buffer<Position>* positions = nullptr;
    Buffer<Color>* colors = nullptr;
    uint32_t nvertices;
    GLuint uniform_offset;

    GLuint compile_shader(const char* vertexShaderSrc,  GLenum);
    GLuint link_program(std::vector<GLuint>& shaders);
    GLuint find_program_attrib(GLuint program, const char* attr);
    GLuint find_program_offset(GLuint program, const char* offset);
	void debug_fill_test_triangle();
    void set_display_window(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
};