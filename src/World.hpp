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
#include "Section.hpp"

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

struct Dimension {
	std::string name;
	bool skylight;
};

void RegisterNewDimension(int dimensionId, const Dimension& newDimension);

class World {
    int dimension = 0;

    std::map<Vector, std::shared_ptr<Section>> sections;

    Section ParseSection(StreamInput *data, const Vector& position);

    std::list<Entity> entities;

    std::vector<Vector> sectionsList;

    void UpdateSectionsList();

public:

	World() = default;

	World(int dimensionId);

    void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

    void ParseChunkData(std::shared_ptr<PacketBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketMultiBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketUnloadChunk> packet);

    bool isPlayerCollides(double X, double Y, double Z) const;

    const std::vector<Vector>& GetSectionsList() const { return sectionsList; }

    const Section &GetSection(const Vector& sectionPos) const;

    RaycastResult Raycast(const glm::vec3& position, const glm::vec3& direction) const;

    void UpdatePhysics(float delta);

    Entity& GetEntity(unsigned int EntityId);

    Entity* GetEntityPtr(unsigned int EntityId);

	const Entity& GetEntity(unsigned int EntityId) const;

    std::vector<unsigned int> GetEntitiesList() const;

    void AddEntity(const Entity& entity);

    void DeleteEntity(unsigned int EntityId);

    BlockId GetBlockId(const Vector& pos) const;

    void SetBlockId(const Vector& pos, BlockId block);

    void SetBlockLight(const Vector& pos, unsigned char light);

    void SetBlockSkyLight(const Vector& pos, unsigned char light);

    const Section *GetSectionPtr(const Vector& position) const;

	unsigned char GetBlockLight(const Vector& pos) const;

	unsigned char GetBlockLight(const Vector& blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const;

	unsigned char GetBlockSkyLight(const Vector& pos) const;

	unsigned char GetBlockSkyLight(const Vector& blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const;
};