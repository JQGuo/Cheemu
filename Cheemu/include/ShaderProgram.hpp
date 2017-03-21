#pragma once

#include <GL/gl3w.h>
#include <vector>

class ShaderProgram {
public:
    ~ShaderProgram();

    void initProgram();
    void addShader( const char * path, GLenum type );
    void linkProgram();

    GLint getUniformLocation(const char * name);

    void enable();
    void disable();

    GLuint program = 0;
    std::vector< GLuint > shaders;
};
