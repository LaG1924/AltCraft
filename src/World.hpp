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
#include "Chunk.hpp"
#include "Section.hpp"
#include "Platform.hpp"

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
struct Biome {
	std::string name;
	float temperature;
	float rainfall;
};

AC_API void RegisterNewDimension(int dimensionId, Dimension newDimension);
AC_API void RegisterNewBiome(int dimensionId, Biome newBiome);

class World {
    int dimension = 0;

	std::map<Vector2I32, std::shared_ptr<Chunk>> chunks;
//    std::map<Vector, std::shared_ptr<Section>> sections;

    Section ParseSection(StreamInput *data, Vector position);

    std::list<Entity> entities;

    std::vector<Vector> sectionsList;

	void UpdateChunkSectionsList();

public:

	World() = default;

	World(int dimensionId);

    void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

    void ParseChunkData(std::shared_ptr<PacketBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketMultiBlockChange> packet);

    void ParseChunkData(std::shared_ptr<PacketUnloadChunk> packet);

    bool isPlayerCollides(double X, double Y, double Z) const;

    std::vector<Vector> GetSectionsList() const;

    const Section &GetSection(Vector sectionPos) const;

	AC_API RaycastResult Raycast(glm::vec3 position, glm::vec3 direction) const;

    void UpdatePhysics(float delta);

    Entity& GetEntity(unsigned int EntityId);

	AC_API Entity* GetEntityPtr(unsigned int EntityId);

	const Entity& GetEntity(unsigned int EntityId) const;

	AC_API std::vector<unsigned int> GetEntitiesList() const;

    void AddEntity(Entity entity);

    void DeleteEntity(unsigned int EntityId);

	AC_API BlockId GetBlockId(Vector pos) const;

	AC_API void SetBlockId(Vector pos, BlockId block);

    void SetBlockLight(Vector pos, unsigned char light);

    void SetBlockSkyLight(Vector pos, unsigned char light);

    const Section *GetSectionPtr(Vector position) const;

	unsigned char GetBlockLight(Vector pos) const;

	unsigned char GetBlockLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const;

	unsigned char GetBlockSkyLight(Vector pos) const;

	unsigned char GetBlockSkyLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const;
};
