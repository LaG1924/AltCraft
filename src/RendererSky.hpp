#pragma once

#include <GL/glew.h>

class RenderState;

class RendererSky {
    GLuint VboVert, VboUv, Vao;
public:
    RendererSky();
    ~RendererSky();
    void Render(RenderState &renderState);
};