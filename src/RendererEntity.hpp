#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"
#include "Entity.hpp"
#include "World.hpp"

class RendererEntity :Renderer {
    unsigned int entityId;
    World *world;
public:
    RendererEntity(World *ptr, unsigned int id);
    ~RendererEntity();

    void Render(RenderState& renderState) override;
    void PrepareResources() override;
    void PrepareRender() override;
    bool IsNeedResourcesPrepare() override;

    GLint modelLoc = 0;
    GLint colorLoc = 0;
};