#pragma once

#include <vector>
#include <array>

#include <glm/mat4x4.hpp>

#include "Block.hpp"
#include "Vector.hpp"

class World;
class BlockModel;

struct RendererSectionData {
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> textures;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> lights;
    size_t hash;
    Vector sectionPos;

    RendererSectionData(World *world, Vector sectionPosition);

    ~RendererSectionData() = default;

    RendererSectionData(RendererSectionData &&other) = default;
private:

    void AddFacesByBlockModel(const std::vector<Vector> &sectionsList, World *world, Vector blockPos, const BlockModel &model, glm::mat4 transform, unsigned char light, unsigned char skyLight, const std::array<unsigned char, 16 * 16 * 16>& visibility);

    std::array<unsigned char, 16 * 16 * 16> GetBlockVisibilityData(World *world);

    std::vector<std::pair<BlockId, const BlockModel *>> idModels;
    const BlockModel* GetInternalBlockModel(const BlockId& id);

    std::array<BlockId, 4096> blockIdData;
    void SetBlockIdData(World *world);

    inline const BlockId& GetBlockId(const Vector& pos) {
        return blockIdData[pos.y * 256 + pos.z * 16 + pos.x];
    }

    inline const BlockId& GetBlockId(int x, int y, int z) {
        return blockIdData[y * 256 + z * 16 + x];
    }
};
