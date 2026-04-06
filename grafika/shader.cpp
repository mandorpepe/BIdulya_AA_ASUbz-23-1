#include "shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void checkCompileErrors(GLuint shader, const std::string& type)
{
    GLint success;
    GLchar infoLog[1024];

    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type" << type << ")\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
        }
    }
}

GLuint createShaderProgram(const std::string& vertexPath,
    const std::string& fragmentPath)
{
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    const char* vertexSrc = vertexCode.c_str();
    const char* fragmentSrc = fragmentCode.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, NULL);
    glCompileShader(vs);
    checkCompileErrors(vs, "VERTEX");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, NULL);
    glCompileShader(fs);
    checkCompileErrors(fs, "FRAGMENT");

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    checkCompileErrors(program, "PROGRAM");

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
