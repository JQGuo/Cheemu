#include "ShaderProgram.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#include "Exception.hpp"
#include "GlErrorCheck.hpp"

using namespace std;

ShaderProgram::~ShaderProgram()
{
    for( auto& s : shaders ) {
        glDeleteShader( s );
    }

    glDeleteProgram( program );
}

void ShaderProgram::initProgram()
{
    program = glCreateProgram();
}

void ShaderProgram::addShader( const char * path, GLenum type )
{ 
    std::stringstream strbuf;

    std::ifstream file;
    file.exceptions(std::ios::failbit | std::ios::badbit);

    file.open( path );
    strbuf << file.rdbuf() << endl;
    file.close();

    // must copy the contents into a new string
    std::string srcStr = strbuf.str();
    const char * srcCstr = srcStr.c_str();

    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, &srcCstr, 0 );
    glCompileShader( shader );

    GLint isCompiled = 0;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &isCompiled );
    if( isCompiled == GL_FALSE )
    {
        GLint maxLength = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

        std::vector< GLchar > infoLog( maxLength );
        glGetShaderInfoLog( shader, maxLength, &maxLength, &infoLog[ 0 ] );

        glDeleteShader( shader );

        stringstream errorMessage;
        errorMessage << &infoLog[ 0 ];
        throw Exception(errorMessage.str());
    }

    glAttachShader( program, shader );

    CHECK_GL_ERRORS;
}

void ShaderProgram::linkProgram()
{
    glLinkProgram( program );

    GLint isLinked = 0;
    glGetProgramiv( program, GL_LINK_STATUS, &isLinked );
    if( isLinked == GL_FALSE )
    {
        GLint maxLength = 0;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

        std::vector< GLchar > infoLog( maxLength );
        glGetProgramInfoLog( program, maxLength, &maxLength, &infoLog[ 0 ] );

        glDeleteProgram( program );

        stringstream errorMessage;
        errorMessage << &infoLog[ 0 ];
        throw Exception(errorMessage.str());
    }

    for( auto& s : shaders ) {
        glDetachShader( program, s );
        glDeleteShader( s );
    }

    CHECK_GL_ERRORS;
}

GLint ShaderProgram::getUniformLocation(const char * name)
{
    GLint result = glGetUniformLocation( program, (const GLchar *) name );                 

    if (result == -1) {
        stringstream errorMessage;
        errorMessage << "Error obtaining uniform location: " << name;
        throw Exception(errorMessage.str());
    }   

    return result;
}

void ShaderProgram::enable()
{
    glUseProgram( program );
    CHECK_GL_ERRORS;
}

void ShaderProgram::disable()
{
    glUseProgram( 0 );
    CHECK_GL_ERRORS;
}

