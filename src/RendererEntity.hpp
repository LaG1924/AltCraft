#pragma once

#include "Gal.hpp"

class World;

class RendererEntity {
    unsigned int entityId;
    std::shared_ptr<Gal::Pipeline> pipeline;
public:
    RendererEntity(unsigned int id);

    void Render(std::shared_ptr<Gal::Pipeline> pipeline, const World *world);
};
