#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <easylogging++.h>
#include <GL/glew.h>

class Shader
{
private:
    const GLchar *vertex;
    const GLchar *fragment;
public:
    // Идентификатор программы
    GLuint Program;
    // Конструктор считывает и собирает шейдер
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    // Использование программы
    void Use();

    void Reload();
};