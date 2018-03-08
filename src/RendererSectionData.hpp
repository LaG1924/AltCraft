#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"

class World;

struct RendererSectionData {
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> textures;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> lights;
    size_t hash = 0;
    Vector sectionPos;
	bool forced = false;
};

RendererSectionData ParseSection(World *world, Vector sectionPosition);