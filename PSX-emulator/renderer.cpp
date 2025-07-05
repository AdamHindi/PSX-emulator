// renderer.cpp
#include "renderer.hpp"
#include <stdexcept>
#include <iostream>


#define CHECK      \
    { GLenum e = glGetError();                   \
      if(e) std::cerr << "GL err 0x" << std::hex << e \
                      << " at " << __LINE__ << '\n'; }

Renderer::Renderer()
{
    nvertices = 0;
    /* 1. Initialise SDL video */
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(SDL_GetError());

    /* 2. Ask for an OpenGL 3.3 core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    /* 3. Create window */
    window_ = SDL_CreateWindow("PSX",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 512,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window_)
        throw std::runtime_error(SDL_GetError());

    /* 4. Create GL context */
    glCtx_ = SDL_GL_CreateContext(window_);
    if (!glCtx_)
        throw std::runtime_error(SDL_GetError());


    /* -------- load OpenGL entry-points via glad -------- */
    if (!gladLoadGLLoader(
        reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
    {
        throw std::runtime_error("Failed to initialise glad");
    }
    /* --------------------------------------------------- */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window_);
    SDL_Delay(2000);            // keep the window up for 2 s

    std::cout << "GL  vendor  : " << glGetString(GL_VENDOR) << '\n';
    std::cout << "GL  version : " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	

    vertexShader = compile_shader(kVertexShader330, GL_VERTEX_SHADER);  
    fragmentShader = compile_shader(fsSrc, GL_FRAGMENT_SHADER);  
    std::vector<GLuint> shaders = { vertexShader, fragmentShader };
    program = link_program(shaders);
	glUseProgram(program);

	// Generate our vertex attribute object
	vertexArrayObject = 0;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	// Create buffers for positions and colors
    positions = new Buffer<Position>();
    colors = new Buffer<Color>();
    /* ---- position (ivec2) ------------------------------------ */
    glBindBuffer(GL_ARRAY_BUFFER, positions->name());          //  ADD
    GLuint loc = find_program_attrib(program, "vertex_position");
    glEnableVertexAttribArray(loc);
    glVertexAttribIPointer(loc, 2, GL_SHORT, 0, nullptr);

    /* ---- colour (uvec3) -------------------------------------- */
    glBindBuffer(GL_ARRAY_BUFFER, colors->name());             //  ADD
    loc = find_program_attrib(program, "vertex_color");
    glEnableVertexAttribArray(loc);
    glVertexAttribIPointer(loc, 3, GL_UNSIGNED_BYTE, 0, nullptr);

    uniform_offset = find_program_offset(program, "offset");
    glUniform2i(uniform_offset, 0, 0);

    glBindVertexArray(vertexArrayObject);
    check_gl_errors();                  //  print nothing

    glDrawArrays(GL_TRIANGLES, 0, nvertices);
    check_gl_errors();                  // prints nothing
}

void Renderer::push_triangle(std::vector<Position>* positions, std::vector<Color>* colors)
{
    if (positions->size() != 3 || colors->size() != 3) {
        throw std::runtime_error("Invalid triangle data");
    }
	// Make sure we have enough space for the new triangle
    if (nvertices + 3 > VERTEX_BUFFER_LEN) {
        throw std::runtime_error("Vertex buffer overflow, force draw");
        draw();
	}
        
    // Copy positions and colors into buffers  
    for (int i = 0; i < 3; ++i) {
        this->positions->set(nvertices + i, (*positions)[i]);
        this->colors->set(nvertices + i, (*colors)[i]);
    }
    nvertices += 3;
}

void Renderer::push_quad(std::vector<Position>* positions, std::vector<Color>* colors)
{
    if (positions->size() != 4 || colors->size() != 4) {
        throw std::runtime_error("Invalid triangle data");
    }
    // Make sure we have enough space for the new triangle
    if (nvertices + 6 > VERTEX_BUFFER_LEN) {
        throw std::runtime_error("Vertex buffer overflow, force draw");
        draw();
    }

    // Copy positions and colors into buffers  
    for (int i = 0; i < 3; ++i) {
        this->positions->set(nvertices , (*positions)[i]);
        this->colors->set(nvertices , (*colors)[i]);
		nvertices += 1;
    }
    for (int i = 1; i < 4; ++i) {
        this->positions->set(nvertices, (*positions)[i]);
        this->colors->set(nvertices, (*colors)[i]);
		nvertices += 1;
    }
}

void Renderer::draw() {
        if (nvertices == 0) return; // Nothing to draw
        
            if (nvertices == 0) return;

            /* 1.  Make sure both VBOs are unmapped on GL 3.x */
            glBindBuffer(GL_ARRAY_BUFFER, positions->name());

            GLint mapped = GL_FALSE;
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_MAPPED, &mapped);
            if (mapped == GL_TRUE)
                glUnmapBuffer(GL_ARRAY_BUFFER);

            glBindBuffer(GL_ARRAY_BUFFER, colors->name());
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_MAPPED, &mapped);
            if (mapped == GL_TRUE)
                glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);


            /* 2.  Draw */
            glUseProgram(program);
            glBindVertexArray(vertexArrayObject);
            glDrawArrays(GL_TRIANGLES, 0, nvertices);

            nvertices = 0;
        
}
void Renderer::display()
{
    // Draw the triangles

    draw();
	check_gl_errors();
    // Swap buffers to display the rendered content
    SDL_GL_SwapWindow(window_);
}
GLuint Renderer::compile_shader(const char* shader_src, GLenum shader_type   )
{
    /* 1. Create shader object */
    auto shader = glCreateShader(shader_type);
    /* 2. Set source code */
    glShaderSource(shader, 1, &shader_src, nullptr);
    /* 3. Compile shaders */
    glCompileShader(shader);
    /* 4. Check for errors */
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        throw std::runtime_error("Vertex shader compilation failed");
    }
    return shader;
}
GLuint Renderer::link_program(std::vector<GLuint>& shaders)
{
    /* 1. Create program object */
    GLuint prog = glCreateProgram();
    if (!prog)
        throw std::runtime_error("glCreateProgram failed");
    /* 2. Attach shaders */
    for (GLuint sh : shaders)
        glAttachShader(prog, sh);
    // Link
    glLinkProgram(prog);
    /* 4. Check for errors */
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);

        std::string log(logLen, '\0');
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());
        std::cerr << "Link-log:\n" << log << std::endl;
        /* clean up */
        glDeleteProgram(prog);
        throw std::runtime_error("Program link error:\n" + log);
    }
    return prog;
}

GLuint Renderer::find_program_attrib(GLuint program, const char* attr)
{
    GLuint attrib = glGetAttribLocation(program, attr);
    if (attrib == GL_INVALID_INDEX) {
        throw std::runtime_error("Attribute not found: " + std::string(attr));
    }
    return attrib;
}
GLuint Renderer::find_program_offset(GLuint program, const char* offset_)
{
    GLint offs = glGetUniformLocation(program, offset_);
    if (offs == GL_INVALID_INDEX) {
        throw std::runtime_error("Offset not found:");
    }
    return offs;
}

void Renderer::debug_fill_test_triangle()
{
    /* 3 vertices – PlayStation VRAM coordinates (x=0..1023, y=0..511) */
    const Position p0{ 256, 400 };      // bottom-left
    const Position p1{ 768, 400 };      // bottom-right
    const Position p2{ 512, 100 };      // top

    const Color    red{ 255,  50,  50 };
    const Color    grn{ 50, 255,  50 };
    const Color    blu{ 50,  50, 255 };

    /* write into persistently-mapped buffers */
    positions->set(0, p0);
    positions->set(1, p1);
    positions->set(2, p2);

    colors->set(0, red);
    colors->set(1, grn);
    colors->set(2, blu);

    nvertices = 3;          // just one triangle
    
}
Renderer::~Renderer()
{
    /*------------------------------------------------------------*
     * 1.  Delete GPU objects while the context is still current  *
     *------------------------------------------------------------*/
    if (vertexArrayObject != 0)
        glDeleteVertexArrays(1, &vertexArrayObject);

    /* VBOs created by Buffer<T> are freed inside Buffer<T>::~Buffer(),
       so you don’t have to touch posVBO_/colVBO_ here.              */

    if (vertexShader != 0)
        glDeleteShader(vertexShader);

    if (fragmentShader != 0)
        glDeleteShader(fragmentShader);

    if (program != 0)
        glDeleteProgram(program);

    /*------------------------------------------------------------*
     * 2.  Tear down the SDL / GL infrastructure                   *
     *------------------------------------------------------------*/
    if (glCtx_)
        SDL_GL_DeleteContext(glCtx_);

    if (window_)
        SDL_DestroyWindow(window_);

    SDL_Quit();
}

void Renderer::set_draw_offset(int16_t x, int16_t y) {
    draw();
    glUniform2i(uniform_offset, static_cast<GLint>(x), static_cast<GLint>(y));
}
void Renderer::set_display_window(uint16_t x0, uint16_t y0,
    uint16_t width, uint16_t height)
{
    if (width == 0 || height == 0) return;
    /* convert PSX coords to actual window coords if you have scaling */
    glViewport(x0, y0, width, height);
    
}