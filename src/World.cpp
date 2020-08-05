#include "World.hpp"

#include <bitset>
#include <glm/glm.hpp>
#include <optick.h>

#include "Event.hpp"
#include "DebugInfo.hpp"
#include "Packet.hpp"
#include "Collision.hpp"

static std::map<int, Dimension> registeredDimensions;
static std::map<int, Biome> registeredBiomes;

void RegisterNewDimension(int dimensionId, Dimension newDimension) {
	registeredDimensions[dimensionId] = newDimension;
}
void RegisterNewBiome(int biomeId, Biome newBiome) {
	newBiome.temperature = std::max(std::min(newBiome.temperature, 1.f), 0.f);
	registeredBiomes[biomeId] = newBiome;
}

World::World(int dimensionId) : dimension(dimensionId) {

}

void World::ParseChunkData(std::shared_ptr<PacketChunkData> packet) {
	StreamBuffer chunkData(packet->Data.data(), packet->Data.size());
	std::bitset<16> bitmask(packet->PrimaryBitMask);

	Vector2I32 chunkPosition = Vector2I32(packet->ChunkX, packet->ChunkZ);
	std::shared_ptr<Chunk> chunk;

	auto it = chunks.find(chunkPosition);
	if (it != chunks.end()) {
		chunk = it->second;
		if (packet->GroundUpContinuous)
			LOG(ERROR) << "New chunk not created " << chunkPosition << " potential memory leak";
	} else {
		chunk = std::make_shared<Chunk>(Vector2I32(packet->ChunkX, packet->ChunkZ), registeredDimensions[dimension].skylight);
		chunks.insert(std::make_pair(chunkPosition, chunk));
	}
	chunk->ParseChunk(packet.get());

	if (packet->GroundUpContinuous)
		UpdateChunkSectionsList();

	for (int i = 0; i < 16; i++)
		if (bitmask[i])
			PUSH_EVENT("ChunkChanged", Vector(chunkPosition.x, i, chunkPosition.z));


}

std::vector<Vector> World::GetSectionsList() const {
	return sectionsList;
}

static Section fallbackSection;

const Section &World::GetSection(Vector position) const {

	if (position.y > 16||position.y < 0)
		return fallbackSection;

	auto it = chunks.find(Vector2I32(position.x, position.z));

	if (it == chunks.end())
		return fallbackSection;

	Section *sec=it->second->GetSection(position.y);
	if (!sec)
		return fallbackSection;

	return *sec;
}

// TODO: skip liquid blocks
RaycastResult World::Raycast(glm::vec3 position, glm::vec3 direction) const {
	OPTICK_EVENT();
    const float maxLen = 5.0;
    const float step = 0.01;
	glm::vec3 pos = glm::vec3(0.0);
    float len = 0;
    Vector blockPos = Vector(position.x,position.y,position.z);
    while (GetBlockId(blockPos) == BlockId{0, 0} && len <= maxLen) {
        pos = position + direction * len;
        len += step;
        blockPos = Vector(floor(pos.x), floor(pos.y), floor(pos.z));
    }

    RaycastResult result;
    result.isHit = !(GetBlockId(blockPos) == BlockId{0, 0});
    result.hitPos = VectorF(pos.x,pos.y,pos.z);
    result.hitBlock = blockPos;
    return result;
}

void World::UpdatePhysics(float delta) {
	OPTICK_EVENT();
    struct CollisionResult {
        bool isCollide;
        //Vector block;
        //VectorF pos; 
        //VectorF dir;
    };

	auto testCollision = [this](double width, double height, VectorF pos)->CollisionResult {
		OPTICK_EVENT("testCollision");
		int blockXBegin = pos.x - width - 1.0;
		int blockXEnd = pos.x + width + 0.5;
		int blockYBegin = pos.y - 0.5;
		int blockYEnd = pos.y + height + 0.5;
		int blockZBegin = pos.z - width - 0.5;
		int blockZEnd = pos.z + width + 0.5;

		AABB entityCollBox;
		entityCollBox.x = pos.x - width / 2.0;
		entityCollBox.y = pos.y;
		entityCollBox.z = pos.z - width / 2.0;

		entityCollBox.w = width;
		entityCollBox.h = height;
		entityCollBox.l = width;

		for (int y = blockYBegin; y <= blockYEnd; y++) {
			for (int z = blockZBegin; z <= blockZEnd; z++) {
				for (int x = blockXBegin; x <= blockXEnd; x++) {
					OPTICK_EVENT("testCollision");
					BlockId block = this->GetBlockId(Vector(x, y, z));
					if (block.id == 0 || !GetBlockInfo(block).collides)
						continue;
					AABB blockColl{ x,y,z,1.0,1.0,1.0 };
					if (TestCollision(entityCollBox, blockColl)) {
						return { true };
					}
				}
			}
		}
		return { false };
	};

    for (auto& it : entities) {
		OPTICK_EVENT("Foreach entities");
		if (it.isFlying) {
			VectorF newPos = it.pos + VectorF(it.vel.x, it.vel.y, it.vel.z) * delta;
			auto coll = testCollision(it.width, it.height, newPos);
			if (coll.isCollide) {
				it.vel = VectorF(0, 0, 0);
			}
			else {
				it.pos = newPos;
			}

			const float AirResistance = 10.0f;
			VectorF resistForce = it.vel * AirResistance * delta * -1.0;
			it.vel = it.vel + resistForce;

			continue;
		}

        { //Vertical velocity
            it.vel.y -= it.gravity * delta;
            VectorF newPos = it.pos + VectorF(0, it.vel.y, 0) * delta;
            auto coll = testCollision(it.width, it.height, newPos);
            if (coll.isCollide) {
                it.vel = VectorF(it.vel.x, 0, it.vel.z);
                it.onGround = true;
            }
            else {
                it.pos = newPos;
            }
        }

        { //Horizontal velocity
            VectorF newPos = it.pos + VectorF(it.vel.x, 0, it.vel.z) * delta;
            auto coll = testCollision(it.width, it.height, newPos);
            if (coll.isCollide) {
                it.vel = VectorF(0, it.vel.y, 0);
            }
            else {
                it.pos = newPos;
            }

            const float AirResistance = 10.0f;
            VectorF resistForce = it.vel * AirResistance * delta * -1.0;
            resistForce.y = 0.0;
            it.vel = it.vel + resistForce;
        }
    }
	DebugInfo::totalChunks = chunks.size();
}

Entity& World::GetEntity(unsigned int EntityId){
    for (auto& it : entities) {
        if (it.entityId == EntityId) {
            return it;
        }
    }

    static Entity fallback;
    return fallback;
}

const Entity &World::GetEntity(unsigned int EntityId) const {
	for (auto& it : entities) {
		if (it.entityId == EntityId) {
			return it;
		}
	}

	static Entity fallback;
	return fallback;
}

std::vector<unsigned int> World::GetEntitiesList() const {
    std::vector<unsigned int> ret;
    for (auto& it : entities) {
        ret.push_back(it.entityId);
    }
    return ret;
}

void World::AddEntity(Entity entity) {
    for (auto& it : entities) {
        if (it.entityId == entity.entityId) {
            LOG(ERROR) << "Adding already existing entity: " << entity.entityId;
            return;
        }
    }
    entities.push_back(entity);
}

void World::DeleteEntity(unsigned int EntityId) {
    auto it = entities.begin();
    for (; it != entities.end(); ++it) {
        if (it->entityId == EntityId) {
            break;
        }
    }
    if (it != entities.end())
        entities.erase(it);
}

void World::ParseChunkData(std::shared_ptr<PacketBlockChange> packet) {
    SetBlockId(packet->Position,
               BlockId {
                   (unsigned short) (packet->BlockId >> 4),
                   (unsigned char) (packet->BlockId & 0xF)
               });

    Vector sectionPos(std::floor(packet->Position.x / 16.0),
                      std::floor(packet->Position.y / 16.0),
                      std::floor(packet->Position.z / 16.0));
    PUSH_EVENT("ChunkChanged", sectionPos);
}

void World::ParseChunkData(std::shared_ptr<PacketMultiBlockChange> packet) {
	Vector2I32 chunkPosition = Vector2I32(packet->ChunkX, packet->ChunkZ);

	auto it = chunks.find(chunkPosition);
	if (it != chunks.end()) {
		std::shared_ptr<Chunk> chunk = it->second;
		chunk->ParseChunkData(packet.get());
	}else
		LOG(ERROR) << "Multi block change of chunk " << chunkPosition;
}

void World::ParseChunkData(std::shared_ptr<PacketUnloadChunk> packet) {
	auto chunk=chunks.find(Vector2I32(packet->ChunkX, packet->ChunkZ));
	if(chunk == chunks.end())
		return;
	chunk->second->Unload();
	chunks.erase(chunk);
	UpdateChunkSectionsList();
}

void World::UpdateChunkSectionsList() {
	sectionsList.clear();
	for (auto& it : chunks) {
		Vector2I32 chunk_coord = it.second->GetPosition();
		Section *section;
		for (unsigned char i = 0; i < 16; i++){
			section=it.second->GetSection(i);
			if(section)
				sectionsList.push_back(Vector(chunk_coord.x, i, chunk_coord.z));
		}
	}
}

BlockId World::GetBlockId(Vector pos) const {
	Vector sectionPos;
	if (pos.y < 0 || pos.y > 255)
		return BlockId{0, 0};
	floorASRQ(pos.x, 4, sectionPos.x);
	floorASRQ(pos.y, 4, sectionPos.y);
	floorASRQ(pos.z, 4, sectionPos.z);

    const Section* section = GetSectionPtr(sectionPos);
    return !section ? BlockId{0, 0} : section->GetBlockId(pos - (sectionPos * 16));
}

void World::SetBlockId(Vector pos, BlockId block) {
	Vector sectionPos;
	floorASRQ(pos.x, 4, sectionPos.x);
	floorASRQ(pos.y, 4, sectionPos.y);
	floorASRQ(pos.z, 4, sectionPos.z);

	Vector blockPos = Vector(pos.x - (sectionPos.x * 16), pos.y, pos.z - (sectionPos.z * 16));

	auto it = chunks.find(Vector2I32(sectionPos.x, sectionPos.z));
	if (it == chunks.end())
		return;

	std::shared_ptr<Chunk> chunk = it->second;

	chunk->SetBlockId(blockPos, block);
}

void World::SetBlockLight(Vector pos, unsigned char light) {

}

void World::SetBlockSkyLight(Vector pos, unsigned char light) {

}

const Section *World::GetSectionPtr(Vector position) const {
	if (position.y < 0 || position.y > 15)
		return nullptr;

	auto it = chunks.find(Vector2I32(position.x, position.z));

	if (it == chunks.end())
        return nullptr;

	return it->second->GetSection(position.y);
}

Entity* World::GetEntityPtr(unsigned int EntityId) {
    for (auto& it : entities) {
        if (it.entityId == EntityId) {
            return &it;
        }
    }
    return nullptr;
}

unsigned char World::GetBlockLight(Vector pos) const {
	Vector sectionPos(std::floor(pos.x / 16.0),
		std::floor(pos.y / 16.0),
		std::floor(pos.z / 16.0));

	Vector blockPos = pos - (sectionPos * 16);

	const Section* section = GetSectionPtr(sectionPos);
	const Section* yp = GetSectionPtr(sectionPos + Vector(0, 1, 0));
	const Section* yn = GetSectionPtr(sectionPos + Vector(0, -1, 0));
	const Section* xp = GetSectionPtr(sectionPos + Vector(1, 0, 0));
	const Section* xn = GetSectionPtr(sectionPos + Vector(-1, 0, 0));
	const Section* zp = GetSectionPtr(sectionPos + Vector(0, 0, 1));
	const Section* zn = GetSectionPtr(sectionPos + Vector(0, 0, -1));

	if (!section)
		return 0;

	return GetBlockLight(blockPos, section, xp, xn, yp, yn, zp, zn);
}

unsigned char World::GetBlockLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const {
	static const Vector directions[] = {
		Vector(0,0,0),
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char value = 0;

	for (const Vector &dir : directions) {
		Vector vec = blockPos + dir;
		unsigned char dirValue = 0;

		if (vec.x < 0 || vec.x > 15 || vec.y < 0 || vec.y > 15 || vec.z < 0 || vec.z > 15) {
			if (vec.x < 0 && xn)
				dirValue = xn->GetBlockLight(Vector(15, vec.y, vec.z));
			if (vec.x > 15 && xp)
				dirValue = xp->GetBlockLight(Vector(0, vec.y, vec.z));
			if (vec.y < 0 && yn)
				dirValue = yn->GetBlockLight(Vector(vec.x, 15, vec.z));
			if (vec.y > 15 && yp)
				dirValue = yp->GetBlockLight(Vector(vec.x, 0, vec.z));
			if (vec.z < 0 && zn)
				dirValue = zn->GetBlockLight(Vector(vec.x, vec.y, 15));
			if (vec.z > 15 && zp)
				dirValue = zp->GetBlockLight(Vector(vec.x, vec.y, 0));
		} else
			dirValue = section->GetBlockLight(vec);

		value = _max(value, dirValue);
	}
	return value;
}

unsigned char World::GetBlockSkyLight(Vector pos) const {
	Vector sectionPos(	std::floor(pos.x / 16.0),
						std::floor(pos.y / 16.0), 
						std::floor(pos.z / 16.0));

	Vector blockPos = pos - (sectionPos * 16);

	const Section* section = GetSectionPtr(sectionPos);
	const Section* yp = GetSectionPtr(sectionPos + Vector(0, 1, 0));
	const Section* yn = GetSectionPtr(sectionPos + Vector(0, -1, 0));
	const Section* xp = GetSectionPtr(sectionPos + Vector(1, 0, 0));
	const Section* xn = GetSectionPtr(sectionPos + Vector(-1, 0, 0));
	const Section* zp = GetSectionPtr(sectionPos + Vector(0, 0, 1));
	const Section* zn = GetSectionPtr(sectionPos + Vector(0, 0, -1));

	if (!section)
		return 0;

	return GetBlockSkyLight(blockPos, section, xp, xn, yp, yn, zp, zn);
}

unsigned char World::GetBlockSkyLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn) const {
	static const Vector directions[] = {
		Vector(0,0,0),
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char value = 0;

	for (const Vector &dir : directions) {
		Vector vec = blockPos + dir;
		unsigned char dirValue = 0;

		if (vec.x < 0 || vec.x > 15 || vec.y < 0 || vec.y > 15 || vec.z < 0 || vec.z > 15) {
			if (vec.x < 0 && xn)
				dirValue = xn->GetBlockSkyLight(Vector(15, vec.y, vec.z));
			if (vec.x > 15 && xp)
				dirValue = xp->GetBlockSkyLight(Vector(0, vec.y, vec.z));
			if (vec.y < 0 && yn)
				dirValue = yn->GetBlockSkyLight(Vector(vec.x, 15, vec.z));
			if (vec.y > 15 && yp)
				dirValue = yp->GetBlockSkyLight(Vector(vec.x, 0, vec.z));
			if (vec.z < 0 && zn)
				dirValue = zn->GetBlockSkyLight(Vector(vec.x, vec.y, 15));
			if (vec.z > 15 && zp)
				dirValue = zp->GetBlockSkyLight(Vector(vec.x, vec.y, 0));
		}
		else
			dirValue = section->GetBlockSkyLight(vec);

		value = _max(value, dirValue);
	}
	return value;
}
