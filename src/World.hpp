#pragma once

#include <map>
#include <queue>
#include <memory>
#include <vector>
#include <list>

#include <easylogging++.h>

#include "Entity.hpp"
#include "Block.hpp"
#include "Vector.hpp"

class Section;
class PacketChunkData;
class PacketBlockChange;
class PacketMultiBlockChange;
class PacketUnloadChunk;
class StreamInput;

struct RaycastResult {
    bool isHit;
    Vector hitBlock;
    VectorF hitPos;
};

class World {
    int dimension = 0;

    std::map<Vector, std::unique_ptr<Section>> sections;

    Section ParseSection(StreamInput *data, Vector position);

    std::list<Entity> entities;

    std::mutex entitiesMutex;

    std::vector<Vector> sectionsList;

    std::mutex sectionsListMutex;

    void UpdateSectionsList();
	std::vector<VectorF> testCollision(double width, double height, VectorF pos);
	bool testCollisionBool(double width, double height, VectorF pos);

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

    RaycastResult Raycast(glm::vec3 position, glm::vec3 direction);

    void UpdatePhysics(float delta);

    Entity& GetEntity(unsigned int EntityId);

    Entity* GetEntityPtr(unsigned int EntityId);

    std::vector<unsigned int> GetEntitiesList();

    void AddEntity(Entity entity);

    void DeleteEntity(unsigned int EntityId);

    BlockId GetBlockId(Vector pos);

    void SetBlockId(Vector pos, BlockId block);    

    void SetBlockLight(Vector pos, unsigned char light);

    void SetBlockSkyLight(Vector pos, unsigned char light);

    Section *GetSectionPtr(Vector position);

	unsigned char GetBlockLight(Vector pos);

	unsigned char GetBlockLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn);

	unsigned char GetBlockSkyLight(Vector pos);

	unsigned char GetBlockSkyLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn);
};
