#pragma once

#include <map>
#include <bitset>
#include <queue>
#include <memory>

#include <easylogging++.h>


#include "Entity.hpp"
#include "Block.hpp"
#include "Section.hpp"
#include "Packet.hpp"
#include "Collision.hpp"
#include "Vector.hpp"

class World {
    int dimension = 0;

    std::map<Vector, std::unique_ptr<Section>> sections;

    Section ParseSection(StreamInput *data, Vector position);

    std::vector<Entity> entities;

    std::mutex entitiesMutex;

    std::vector<Vector> sectionsList;

    std::mutex sectionsListMutex;

    void UpdateSectionsList();

public:
    World();

    ~World();

    void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

    void ParseChunkData(std::shared_ptr<PacketBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketMultiBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketUnloadChunk> packet);

    bool isPlayerCollides(double X, double Y, double Z);

    std::vector<Vector> GetSectionsList();

    const Section &GetSection(Vector sectionPos);

    glm::vec3 Raycast(glm::vec3 position, glm::vec3 direction, float maxLength = 1000.0f, float minPrecision = 0.01f);

    void UpdatePhysics(float delta);

    Entity& GetEntity(unsigned int EntityId);

    std::vector<unsigned int> GetEntitiesList();

    void AddEntity(Entity entity);

    void DeleteEntity(unsigned int EntityId);

    BlockId GetBlockId(Vector pos);

    void SetBlockId(Vector pos, BlockId block);    

    void SetBlockLight(Vector pos, unsigned char light);

    void SetBlockSkyLight(Vector pos, unsigned char light);

    Section *GetSectionPtr(Vector position);
};