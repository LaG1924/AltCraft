#pragma once

#include <GL/glew.h>

class Shader
{
private:
    const GLchar *vertex;
    const GLchar *fragment;
public:
    GLuint Program;
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);
    void Use();

    void Reload();
};