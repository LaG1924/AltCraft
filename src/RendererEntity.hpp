#pragma once

#include <GL/glew.h>

class RenderState;
class World;

class RendererEntity {
    unsigned int entityId;
public:
    RendererEntity(unsigned int id);
    ~RendererEntity();

    void Render(RenderState& renderState, const World *world);

    static GLuint GetVao();
};
