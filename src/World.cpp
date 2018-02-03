#include "World.hpp"

#include <bitset>
#include <glm/glm.hpp>

#include "Section.hpp"
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
            Section section = ParseSection(&chunkData, chunkPosition);

            if (packet->GroundUpContinuous) {
                if (!sections.insert(std::make_pair(chunkPosition, std::make_unique<Section>(section))).second) {
                    LOG(ERROR) << "New chunk not created " << chunkPosition << " potential memory leak";
                }
                UpdateSectionsList();
            }
            else {
                using std::swap;
                swap(*sections.at(chunkPosition).get(), section);
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


    return Section(position, bitsPerBlock, std::move(palette), std::move(blockArray), std::move(blockLight), std::move(skyLight));
}

World::~World() {
}

World::World() {
}

bool World::isPlayerCollides(double X, double Y, double Z) {
    Vector PlayerChunk(floor(X / 16.0), floor(Y / 16.0), floor(Z / 16.0));
    if (sections.find(PlayerChunk) == sections.end() || sections.find(PlayerChunk - Vector(0, 1, 0)) == sections.end())
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
                    if (block.id == 0 || block.id == 31)
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

std::vector<Vector> World::GetSectionsList() {
    sectionsListMutex.lock();
    auto vec = sectionsList;
    sectionsListMutex.unlock();
    return vec;
}

static Section fallbackSection;

const Section &World::GetSection(Vector sectionPos) {
    auto result = sections.find(sectionPos);
    if (result == sections.end()) {
         //LOG(ERROR) << "Accessed not loaded section " << sectionPos;
        return fallbackSection;
    }
    else {
        return *result->second.get();
    }
}

Vector World::Raycast(glm::vec3 position, glm::vec3 direction, float &distance) {
    auto triangle_intersection = [&] (const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) -> bool {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;

        glm::vec3 pvec = glm::cross(direction, e2);

        float det = glm::dot(e1, pvec);
        if (det < 1e-8 && det > -1e-8) {
            return 0;
        }

        float inv_det = 1 / det;
        glm::vec3 tvec = position - v0;
        float u = dot(tvec, pvec) * inv_det;
        if (u < 0 || u > 1) {
            return 0;
        }

        glm::vec3 qvec = cross(tvec, e1);
        float v = dot(direction, qvec) * inv_det;
        if (v < 0 || u + v > 1) {
            return 0;
        }
        return dot(e2, qvec) * inv_det;
    };

    float minDistance = 1000000;
    Vector minBlock;

    for (int y = position.y-5; y<position.y+5;y++) {
        for (int z = position.z-5;z<position.z+5;z++) {
            for (int x = position.x-5;x<position.x+5;x++) {
                if (GetBlockId(Vector(x,y,z)) == BlockId{0,0})
                    continue;

                //Z- north
                //Z+ south
                //X+ east
                //X- west
                //Y+ top
                //Y- bottom
                glm::vec3 vNNN {x,y,z};
                glm::vec3 vNNP {x,y,z+1};
                glm::vec3 vNPN {x,y+1,z};
                glm::vec3 vNPP {x,y+1,z+1};
                glm::vec3 vPNN {x+1,y,z};
                glm::vec3 vPNP {x+1,y,z+1};
                glm::vec3 vPPN {x+1,y+1,z};
                glm::vec3 vPPP {x+1,y+1,z+1};

                float west = triangle_intersection(vNNN,vNPN,vNPP);
                float east = triangle_intersection(vPPN,vPNP,vPNN);
                float north = triangle_intersection(vNPN,vPNN,vNNN);
                float south = triangle_intersection(vNNP,vNPP,vPNP);
                float top = triangle_intersection(vNPN,vNPP,vPPN);
                float bottom = triangle_intersection(vNNN,vNNP,vPNN);

                if (west || east || north || south || top || bottom) {
                    float len = (Vector(position.x,position.y,position.z) - Vector(x,y,z)).GetLength();
                    if (len <= minDistance) {
                        float we = west < east && west != 0 ? west : east;
                        float ns = north < south && north != 0 ? north : south;
                        float tb = top < bottom && top != 0 ? top : bottom;
                        float wens = we < ns && we != 0 ? we : ns;
                        minDistance = wens < tb && wens != 0? wens : tb;
                        minBlock = Vector(x,y,z);
                    }
                }
            }
        }
    }

    if (minDistance == 1000000) {
        distance = 0;
        return Vector(0,0,0);
    }

    distance = minDistance;
    return minBlock;
}

void World::UpdatePhysics(float delta)
{
    struct CollisionResult {
        bool isCollide;
        //Vector block;
        //VectorF pos; 
        //VectorF dir;
    };

    auto testCollision = [this](double width, double height, VectorF pos)->CollisionResult {

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
                    BlockId block = this->GetBlockId(Vector(x, y, z));
                    if (block.id == 0 || block.id == 31)
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

    entitiesMutex.lock();
    for (auto& it : entities) {
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
    entitiesMutex.unlock();
    DebugInfo::totalSections = sections.size();
}

Entity & World::GetEntity(unsigned int EntityId)
{
    entitiesMutex.lock();
    for (auto& it : entities) {
        if (it.entityId == EntityId) {
            entitiesMutex.unlock();
            return it;
        }
    }
    entitiesMutex.unlock();
    static Entity fallback;
    return fallback;
}

std::vector<unsigned int> World::GetEntitiesList()
{
    entitiesMutex.lock();
    std::vector<unsigned int> ret;
    for (auto& it : entities) {
        ret.push_back(it.entityId);
    }
    entitiesMutex.unlock();
    return ret;
}

void World::AddEntity(Entity entity)
{
    entitiesMutex.lock();
    for (auto& it : entities) {
        if (it.entityId == entity.entityId) {
            LOG(ERROR) << "Adding already existing entity: " << entity.entityId;
            entitiesMutex.unlock();
            return;
        }
    }
    entities.push_back(entity);
    entitiesMutex.unlock();
}

void World::DeleteEntity(unsigned int EntityId)
{
    entitiesMutex.lock();
    auto it = entities.begin();
    for (; it != entities.end(); ++it) {
        if (it->entityId == EntityId) {
            break;
        }
    }
    if (it != entities.end())
        entities.erase(it);
    entitiesMutex.unlock();
}

void World::ParseChunkData(std::shared_ptr<PacketBlockChange> packet) {
    SetBlockId(packet->Position, BlockId{(unsigned short) (packet->BlockId >> 4),(unsigned char) (packet->BlockId & 0xF) });

    Vector sectionPos(std::floor(packet->Position.x / 16.0), std::floor(packet->Position.y / 16.0), std::floor(packet->Position.z / 16.0));
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
    std::vector<std::map<Vector, std::unique_ptr<Section>>::iterator> toRemove;
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
    sectionsListMutex.lock();
    sectionsList.clear();
    for (auto& it : sections) {
        sectionsList.push_back(it.first);
    }
    sectionsListMutex.unlock();
}

BlockId World::GetBlockId(Vector pos) {
    Vector sectionPos(std::floor(pos.x / 16.0), std::floor(pos.y / 16.0), std::floor(pos.z / 16.0));

    Section* section = GetSectionPtr(sectionPos);
    return !section ? BlockId{0, 0} : section->GetBlockId(pos - (sectionPos * 16));
}

void World::SetBlockId(Vector pos, BlockId block) {
    Vector sectionPos(std::floor(pos.x / 16.0), std::floor(pos.y / 16.0), std::floor(pos.z / 16.0));
    Section* section = GetSectionPtr(sectionPos);
    section->SetBlockId(pos - (sectionPos * 16), block);
    PUSH_EVENT("ChunkChanged",sectionPos);
}

void World::SetBlockLight(Vector pos, unsigned char light) {

}

void World::SetBlockSkyLight(Vector pos, unsigned char light) {

}

Section *World::GetSectionPtr(Vector position) {
    auto it = sections.find(position);

    if (it == sections.end())
        return nullptr;

    return it->second.get();
}

Entity* World::GetEntityPtr(unsigned int EntityId) {
    entitiesMutex.lock();
    for (auto& it : entities) {
        if (it.entityId == EntityId) {
            entitiesMutex.unlock();
            return &it;
        }
    }
    entitiesMutex.unlock();
    return nullptr;
}