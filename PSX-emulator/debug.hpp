// gl_debug_utils.hpp
#pragma once
#include <glad/glad.h>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// -----------------------------------------------------------------------------
// Helpers that convert OpenGL enum values into short strings
// -----------------------------------------------------------------------------
inline const char* debug_source_str(GLenum s)
{
    switch (s) {
    case GL_DEBUG_SOURCE_API:             return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:     return "3RD";
    case GL_DEBUG_SOURCE_APPLICATION:     return "APP";
    case GL_DEBUG_SOURCE_OTHER:           return "OTHER";
    default:                              return "UNKNOWN";
    }
}
inline const char* debug_type_str(GLenum t)
{
    switch (t) {
    case GL_DEBUG_TYPE_ERROR:               return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEF";
    case GL_DEBUG_TYPE_PORTABILITY:         return "PORT";
    case GL_DEBUG_TYPE_PERFORMANCE:         return "PERF";
    case GL_DEBUG_TYPE_MARKER:              return "MARK";
    case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH";
    case GL_DEBUG_TYPE_POP_GROUP:           return "POP";
    case GL_DEBUG_TYPE_OTHER:               return "OTHER";
    default:                                return "UNKNOWN";
    }
}
inline const char* debug_severity_str(GLenum s)
{
    switch (s) {
    case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM:       return "MED";
    case GL_DEBUG_SEVERITY_LOW:          return "LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTE";
    default:                             return "UNKW";
    }
}

// -----------------------------------------------------------------------------
//  check_gl_errors()
//      • prints every pending debug message
//      • throws std::runtime_error if a HIGH-severity message is found
// -----------------------------------------------------------------------------
inline void check_gl_errors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        std::cerr << "legacy GL error 0x" << std::hex << err << '\n';
}