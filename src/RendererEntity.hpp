#pragma once

#include <GL/glew.h>

class RenderState;

class RendererEntity {
    unsigned int entityId;
public:
    RendererEntity(unsigned int id);
    ~RendererEntity();

    void Render(RenderState& renderState);

    static GLuint GetVao();
};
