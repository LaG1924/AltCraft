#pragma once

#include <GL/glew.h>

class World;
class RenderState;

class RendererEntity {
    unsigned int entityId;
    World *world;
public:
    RendererEntity(World *ptr, unsigned int id);
    ~RendererEntity();

    void Render(RenderState& renderState);

    static GLuint GetVao();
};
