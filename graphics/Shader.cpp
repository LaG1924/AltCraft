#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include "Shader.hpp"

Shader::Shader(std::string fileName, bool vertex) {
    this->isVertex = vertex;
    std::ifstream in(fileName);
    if (!in){
        std::cout<<"Can't open shader source at "<<fileName<<std::endl;
        throw 519;
    }
    shaderSource = std::string((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
    shaderId = glCreateShader(isVertex?GL_VERTEX_SHADER:GL_FRAGMENT_SHADER);
    const char* shaderSrc = shaderSource.c_str();
    glShaderSource(shaderId, 1, &shaderSrc, NULL);
    glCompileShader(shaderId);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        throw 518;
    }
}

Shader::~Shader() {
    glDeleteShader(shaderId);
}

void Shader::bind() {

}
