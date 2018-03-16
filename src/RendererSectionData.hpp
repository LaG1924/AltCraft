#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "Section.hpp"

class World;

struct SectionsData {
	Section section;
	Section west;
	Section east;
	Section top;
	Section bottom;
	Section north;
	Section south;
	
	unsigned char GetLight(const Vector &pos) const;

	unsigned char GetSkyLight(const Vector &pos) const;
};

struct RendererSectionData {
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> textures;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> lights;
    size_t hash = 0;
    Vector sectionPos;
	bool forced = false;
};

RendererSectionData ParseSection(const SectionsData &sections);