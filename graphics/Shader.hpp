#pragma once

#include <string>

class Shader {
    std::string shaderSource;
    GLuint shaderId;
    bool isVertex = true;

    Shader(const Shader &);

public:
    Shader(std::string fileName, bool vertex = true);

    ~Shader();

    void bind();

    GLuint GetId() {
        return shaderId;
    }

};

class ShaderProgram {
    GLuint shaderProgram;
public:
    ShaderProgram() {
        shaderProgram = glCreateProgram();
    }

    ~ShaderProgram() {
        glDeleteProgram(shaderProgram);
    }

    void Attach(Shader &shader) {
        glAttachShader(shaderProgram, shader.GetId());
    }

    void Link() {
        glLinkProgram(shaderProgram);
        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "Shader program linking failed: " << infoLog << std::endl;
        }
        glUseProgram(shaderProgram);
    }

    GLuint GetId() {
        return shaderProgram;
    }

    explicit operator GLuint() const {
        return shaderProgram;
    }

};