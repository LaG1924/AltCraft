#pragma once

#include <easylogging++.h>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>

class Texture {
    Texture(Texture&);
    Texture&operator=(Texture&);
public:
    GLuint texture;
    Texture(std::string filename, GLenum textureWrapping = GL_CLAMP_TO_BORDER, GLenum textureFiltering = GL_NEAREST);
    ~Texture();
};