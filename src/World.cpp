#include "World.hpp"

#include <bitset>
#include <glm/glm.hpp>
#include <optick.h>

#include "Event.hpp"
#include "DebugInfo.hpp"
#include "Packet.hpp"
#include "Collision.hpp"

void World::ParseChunkData(std::shared_ptr<PacketChunkData> packet) {
    StreamBuffer chunkData(packet->Data.data(), packet->Data.size());
    std::bitset<16> bitmask(packet->PrimaryBitMask);
    for (int i = 0; i < 16; i++) {
        if (bitmask[i]) {
            Vector chunkPosition = Vector(packet->ChunkX, i, packet->ChunkZ);
            auto section = std::make_shared<Section>(ParseSection(&chunkData, chunkPosition));

            if (packet->GroundUpContinuous) {
                if (!sections.insert(std::make_pair(chunkPosition, section)).second) {
                    LOG(ERROR) << "New chunk not created " << chunkPosition << " potential memory leak";
                }

                UpdateSectionsList();
            } else {
				std::swap(sections.at(chunkPosition), section);
            }

            PUSH_EVENT("ChunkChanged", chunkPosition);
        }
    }
}

Section World::ParseSection(StreamInput *data, Vector position) {
    unsigned char bitsPerBlock = data->ReadUByte();

    int paletteLength = data->ReadVarInt();
    std::vector<unsigned short> palette;
    for (int i = 0; i < paletteLength; i++) {
        palette.push_back(data->ReadVarInt());
    }
    int dataArrayLength = data->ReadVarInt();
    auto dataArray = data->ReadByteArray(dataArrayLength * 8);
    auto blockLight = data->ReadByteArray(2048);
    std::vector<unsigned char> skyLight;
    if (dimension == 0)
        skyLight = data->ReadByteArray(2048);

    long long *blockData = reinterpret_cast<long long*>(dataArray.data());
    for (int i = 0; i < dataArray.size() / sizeof(long long); i++)
        endswap(blockData[i]);
    std::vector<long long> blockArray(blockData, blockData + dataArray.size() / sizeof(long long));


    return Section(
        position, bitsPerBlock, std::move(palette),std::move(blockArray),
        std::move(blockLight), std::move(skyLight));
}

bool World::isPlayerCollides(double X, double Y, double Z) const {
    Vector PlayerChunk(floor(X / 16.0), floor(Y / 16.0), floor(Z / 16.0));
    if (sections.find(PlayerChunk) == sections.end() ||
        sections.find(PlayerChunk - Vector(0, 1, 0)) == sections.end())
        return false;

    std::vector<Vector> closestSectionsCoordinates = {
        Vector(PlayerChunk.x, PlayerChunk.y, PlayerChunk.z),
        Vector(PlayerChunk.x + 1, PlayerChunk.y, PlayerChunk.z),
        Vector(PlayerChunk.x - 1, PlayerChunk.y, PlayerChunk.z),
        Vector(PlayerChunk.x, PlayerChunk.y + 1, PlayerChunk.z),
        Vector(PlayerChunk.x, PlayerChunk.y - 1, PlayerChunk.z),
        Vector(PlayerChunk.x, PlayerChunk.y, PlayerChunk.z + 1),
        Vector(PlayerChunk.x, PlayerChunk.y, PlayerChunk.z - 1),
    };
    std::vector<Vector> closestSections;
    for (auto &coord : closestSectionsCoordinates) {
        if (sections.find(coord) != sections.end())
            closestSections.push_back(coord);
    }

    for (auto &it : closestSections) {

        const double PlayerWidth = 0.6;
        const double PlayerHeight = 1.82;
        const double PlayerLength = 0.6;

        AABB playerColl;
        playerColl.x = X - PlayerWidth / 2.0;
        playerColl.w = PlayerWidth;
        playerColl.y = Y;
        playerColl.h = PlayerHeight;
        playerColl.z = Z - PlayerLength / 2.0;
        playerColl.l = PlayerLength;

        const Section &section = this->GetSection(it);
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    BlockId block = section.GetBlockId(Vector(x, y, z));
                    if (!GetBlockInfo(block).collides)
                        continue;
                    AABB blockColl{ (x + it.x * 16.0),
                        (y + it.y * 16.0),
                        (z + it.z * 16.0), 1, 1, 1 };
                    if (TestCollision(playerColl, blockColl))
                        return true;
                }
            }
        }
    }
    return false;
}

std::vector<Vector> World::GetSectionsList() const {
    auto vec = sectionsList;
    return vec;
}

static Section fallbackSection;

const Section &World::GetSection(Vector sectionPos) const {
    auto result = sections.find(sectionPos);
    if (result == sections.end()) {
         //LOG(ERROR) << "Accessed not loaded section " << sectionPos;
        return fallbackSection;
    }
    else {
        return *result->second;
    }
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
    DebugInfo::totalSections = sections.size();
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
    std::vector<Vector> changedSections;
    for (auto& it : packet->Records) {
        int x = (it.HorizontalPosition >> 4 & 15) + (packet->ChunkX * 16);
        int y = it.YCoordinate;
        int z = (it.HorizontalPosition & 15) + (packet->ChunkZ * 16);
        Vector worldPos(x, y, z);
        SetBlockId(worldPos, BlockId{(unsigned short) (it.BlockId >> 4),(unsigned char) (it.BlockId & 0xF) });

        Vector sectionPos(packet->ChunkX, std::floor(it.YCoordinate / 16.0), packet->ChunkZ);
        if (std::find(changedSections.begin(), changedSections.end(), sectionPos) == changedSections.end())
            changedSections.push_back(sectionPos);
    }

    for (auto& sectionPos : changedSections)
        PUSH_EVENT("ChunkChanged", sectionPos);
}

void World::ParseChunkData(std::shared_ptr<PacketUnloadChunk> packet) {
    std::vector<std::map<Vector, std::shared_ptr<Section>>::iterator> toRemove;
    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (it->first.x == packet->ChunkX && it->first.z == packet->ChunkZ)
            toRemove.push_back(it);
    }
    for (auto& it : toRemove) {
        PUSH_EVENT("ChunkDeleted", it->first);
        sections.erase(it);
    }
    UpdateSectionsList();
}

void World::UpdateSectionsList() {
    sectionsList.clear();
    for (auto& it : sections) {
        sectionsList.push_back(it.first);
    }
}

BlockId World::GetBlockId(Vector pos) const {
    Vector sectionPos(std::floor(pos.x / 16.0),
                      std::floor(pos.y / 16.0),
                      std::floor(pos.z / 16.0));

    const Section* section = GetSectionPtr(sectionPos);
    return !section ? BlockId{0, 0} : section->GetBlockId(pos - (sectionPos * 16));
}

void World::SetBlockId(Vector pos, BlockId block) {
    Vector sectionPos(std::floor(pos.x / 16.0),
                      std::floor(pos.y / 16.0),
                      std::floor(pos.z / 16.0));
	Vector blockPos = pos - (sectionPos * 16);
	auto section = std::make_shared<Section>(*GetSectionPtr(sectionPos));
    section->SetBlockId(blockPos, block);
	sections[sectionPos] = section;
    PUSH_EVENT("ChunkChanged",sectionPos);
	if (blockPos.x == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(-1, 0, 0));
	if (blockPos.x == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(1, 0, 0));
	if (blockPos.y == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, -1, 0));
	if (blockPos.y == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 1, 0));
	if (blockPos.z == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 0, -1));
	if (blockPos.z == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 0, 1));
}

void World::SetBlockLight(Vector pos, unsigned char light) {

}

void World::SetBlockSkyLight(Vector pos, unsigned char light) {

}

const Section *World::GetSectionPtr(Vector position) const {
    auto it = sections.find(position);

    if (it == sections.end())
        return nullptr;

	return it->second.get();
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
