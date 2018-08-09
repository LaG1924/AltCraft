#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "Section.hpp"
#include "AssetManager.hpp"

class World;

struct BlockLightness {
	unsigned char face[FaceDirection::none] = { 0,0,0,0,0,0 };
};

struct SectionsData {
	Section section;
	Section west;
	Section east;
	Section top;
	Section bottom;
	Section north;
	Section south;
	
	BlockLightness GetLight(const Vector &pos) const;

	BlockLightness GetSkyLight(const Vector &pos) const;
};

struct RendererSectionData {
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> textures;
	std::vector<float> textureLayers;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> lights;
    size_t hash = 0;
    Vector sectionPos;
	bool forced = false;
};

RendererSectionData ParseSection(const SectionsData &sections);