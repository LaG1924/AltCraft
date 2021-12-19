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

	BlockId GetBlockId(const Vector &pos) const;
	
	BlockLightness GetLight(const Vector &pos) const;

	BlockLightness GetSkyLight(const Vector &pos) const;
};

struct VertexData {
    glm::vec3 positions[4];
    glm::vec2 uvs[4];
    glm::vec2 lights[4];
    glm::vec3 normal;
    glm::vec3 colors;
    glm::vec3 layerAnimationAo; //R - uvLayer, G - animation, B - ambientOcclusion
};

struct RendererSectionData {
    std::vector<VertexData> vertices;
    size_t hash = 0;
    Vector sectionPos;
    bool forced = false;
};

RendererSectionData ParseSection(const SectionsData &sections);