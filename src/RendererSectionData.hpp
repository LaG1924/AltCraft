#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "Section.hpp"
#include "AssetManager.hpp"

class World;

struct BlockLightness {
    uint8_t face[FaceDirection::none + 1] = { 0,0,0,0,0,0 };
    uint8_t self = 0;
};

struct SectionsData {
    Section data[3][3][3];

    const Section& GetSection(Vector& pos) const {
        size_t x = 1, y = 1, z = 1;
        while (true) {
            if (pos.x < 0) {
                x--;
                pos.x += 16;
            }
            else if (pos.x > 15) {
                x++;
                pos.x -= 16;
            }
            else if (pos.y < 0) {
                y--;
                pos.y += 16;
            }
            else if (pos.y > 15) {
                y++;
                pos.y -= 16;
            }
            else if (pos.z < 0) {
                z--;
                pos.z += 16;
            }
            else if (pos.z > 15) {
                z++;
                pos.z -= 16;
            }
            else
                break;
        }
        return data[x][y][z];
    }

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

RendererSectionData ParseSection(const SectionsData &sections, bool smoothLighting);
