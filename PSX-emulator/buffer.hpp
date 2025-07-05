#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <cstring>      // std::memset
#include <stdexcept>

/// Maximum number of vertices the attribute buffer can hold
constexpr std::uint32_t VERTEX_BUFFER_LEN = 64 * 1024;   // 64k

/// Write-only buffer with enough space for VERTEX_BUFFER_LEN elements
template<typename T>
class Buffer
{
public:
    Buffer();                       // allocate + map
    ~Buffer();                      // unmap + delete

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    /// Overwrite entry `index` with `val`.
    void set(std::uint32_t index, const T& val);

    /// Return raw pointer in case you need direct access
    T* mapped() noexcept { return map_; }

    /// Return the GL name so you can glBindBuffer elsewhere
    GLuint name() const noexcept { return object_; }


    GLuint object_{ 0 };
    T* map_{ nullptr };
};


template<typename T>
Buffer<T>::Buffer()
{
    /* 1. Create and bind object */
    glGenBuffers(1, &object_);
    glBindBuffer(GL_ARRAY_BUFFER, object_);

    /* 2. Compute byte size */
    constexpr GLsizeiptr elementSize = static_cast<GLsizeiptr>(sizeof(T));
    constexpr GLsizeiptr bufferSize = elementSize * VERTEX_BUFFER_LEN;

    /* 3. Allocate write-only, persistently-mapped storage */
    const GLbitfield access =
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

    /* -------- choose allocation method ---------------------------- */
    if (glBufferStorage)            // fast path (4.4+ / ARB_buffer_storage)
    {
        const GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
        glBufferStorage(GL_ARRAY_BUFFER, bufferSize, nullptr, access);

        map_ = static_cast<T*>(
            glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, access));
    }
    else                            // fallback for GL 3.x (your case)
    {
        glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

        /* Prefer Range if available, else MapBuffer */
        if (glMapBufferRange)
            map_ = static_cast<T*>(
                glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize,
                    GL_MAP_WRITE_BIT));
        else
            map_ = static_cast<T*>(
                glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    }
    if (!map_)
        throw std::runtime_error("glMapBuffer failed");




    /* 5. Clear to zero to avoid uninitialised garbage */
    std::memset(map_, 0, bufferSize);
}

template<typename T>
Buffer<T>::~Buffer()
{
    if (object_)
    {
        glBindBuffer(GL_ARRAY_BUFFER, object_);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &object_);
    }
}

template<typename T>
void Buffer<T>::set(std::uint32_t index, const T& val)
{
    if (index >= VERTEX_BUFFER_LEN)
        throw std::out_of_range("vertex buffer overflow");

    map_[index] = val;              // write directly into persist-map
}
