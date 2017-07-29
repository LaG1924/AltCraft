#pragma once

#include <SFML/Graphics.hpp>
#include <easylogging++.h>
#include <GL/glew.h>

class Texture {
    Texture(Texture&);
    Texture&operator=(Texture&);
public:
    GLuint texture;
    Texture(std::string filename, GLenum textureWrapping = GL_CLAMP_TO_BORDER, GLenum textureFiltering = GL_NEAREST);
    ~Texture();
};