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
            } else {
                std::swap(*sections.at(chunkPosition).get(), section);
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
	for (size_t i = 0; i < dataArray.size() / sizeof(long long); i++)
        endswap(blockData[i]);
    std::vector<long long> blockArray(blockData, blockData + dataArray.size() / sizeof(long long));


    return Section(
        position, bitsPerBlock, std::move(palette),std::move(blockArray),
        std::move(blockLight), std::move(skyLight));
}

World::~World() {
}

World::World() {
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

// TODO: skip liquid blocks
RaycastResult World::Raycast(glm::vec3 position, glm::vec3 direction) {
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
inline bool isUncollideble(int id){
	return id == 0 || id == 31 || id == 37 || id == 38 || id == 175 || id == 78 || id == 55 || id == 69 || id == 75 || id == 76;
}
bool World::testCollisionBool(double width, double height, VectorF pos){
		double pre=width/2;
		int blockXBegin = pos.x - width - pre;
		int blockXEnd = pos.x + width + pre;
		int blockYBegin = pos.y - 0.5;
		int blockYEnd = pos.y + height + 0.5;
		int blockZBegin = pos.z - width - pre;
		int blockZEnd = pos.z + width + pre;

        for (int y = blockYBegin; y <= blockYEnd; y++) {
		 for (int z = blockZBegin; z <= blockZEnd; z++) {
		  for (int x = blockXBegin; x <= blockXEnd; x++) {
			BlockId block = this->GetBlockId(Vector(x, y, z));
			 if (isUncollideble(block.id))
			  continue;
			 if(TestCollisionV(pos.z-pre, width, z, 1.0)&&TestCollisionV(pos.x-pre, width, x, 1.0)&&TestCollisionV(pos.y, height, y, 1.0))
			  return true;
		  }
		 }
		}
		return false;
}
// TODO use static arrays
std::vector<VectorF> World::testCollision(double width, double height, VectorF pos){
		double pre=width/2;
		int blockXBegin = pos.x - width - 0.5;
		int blockXEnd = pos.x + width + 0.5;
		int blockYBegin = pos.y - 0.5;
		int blockYEnd = pos.y + height + 0.5;
		int blockZBegin = pos.z - width - 0.5;
		int blockZEnd = pos.z + width + 0.5;

		std::vector<VectorF> collided;

        for (int y = blockYBegin; y <= blockYEnd; y++) {
            for (int z = blockZBegin; z <= blockZEnd; z++) {
                for (int x = blockXBegin; x <= blockXEnd; x++) {
                    BlockId block = this->GetBlockId(Vector(x, y, z));
					if (isUncollideble(block.id))
						continue;
					if(TestCollisionV(pos.x - pre, width, x, 1.0)&&TestCollisionV(pos.y, height, y, 1.0)&&TestCollisionV(pos.z - pre, width, z, 1.0))
					{
						collided.push_back(VectorF(x,y,z));
					}
                }
            }
        }
        return collided;
}
void World::UpdatePhysics(float delta) {
    entitiesMutex.lock();
    for (auto& it : entities) {
		if (it.isFlying) {
			VectorF newPos = it.pos + VectorF(it.vel.x, it.vel.y, it.vel.z) * delta;
			std::vector<VectorF> collided=testCollision(it.width, it.height, newPos);
			if(collided.empty()) {
				it.pos = newPos;
			}else{
				it.vel = VectorF(0, 0, 0);
			}

			const float AirResistance = 10.0f;
			VectorF resistForce = it.vel * AirResistance * delta * -1.0;
			it.vel = it.vel + resistForce;
			continue;
		}
		{ //Vertical velocity
			if(it.vel.y!=0)
				it.onGround=false;
			it.vel.y -= it.gravity * delta;
			VectorF newPos = it.pos + VectorF(0, it.vel.y, 0) * delta;
			if(testCollisionBool(it.width, it.height, newPos)){
				it.vel = VectorF(it.vel.x, 0, it.vel.z);
				it.onGround = true;
			}else{
				it.pos = newPos;
            }
        }
		{ //X vel
			xre:
			VectorF newPos=it.pos + VectorF(it.vel.x, 0, 0) * delta;
			std::vector<VectorF> collided=testCollision(it.width, it.height, newPos);
			if(collided.empty()){
				it.pos = newPos;
			}else{
				double pre=it.width/2;
				for(size_t i=0; i!=collided.size(); i++){
					VectorF B=collided.at(i);
					if(B.y+1.0==it.pos.y)
						continue;
					if ((it.pos.z+pre > B.z) || (it.pos.z-pre < B.z+1))
					  continue;

					if ((it.vel.x > 0.0) && (it.pos.x+pre <= B.x)){
					  double max = B.x - it.pos.x+pre;
					  if (max < it.vel.x) {
						it.vel.x = max;
					  }
					}else if ((it.vel.x < 0.0) && (it.pos.x-pre >= B.x+1)){
					  double max = B.x+1 - it.pos.x-pre;
					  if (max > it.vel.x)
						it.vel.x = max;
					}else continue;
					goto xre;
				}
			}
		}
		{ //Z vel
				zre:
				VectorF newPos=it.pos + VectorF(0, 0, it.vel.z) * delta;
				std::vector<VectorF> collided=testCollision(it.width, it.height, newPos);
				if(collided.empty()){
					it.pos = newPos;
				}else{
					double pre=it.width/2;
					for(size_t i=0; i!=collided.size(); i++){
						VectorF B=collided.at(i);
						if(B.y+1.0==it.pos.y)
							continue;
						if ((it.pos.x+pre > B.x) || (it.pos.x-pre < B.x+1))
						  continue;
						if ((it.vel.z > 0.0) && (it.pos.z+pre <= B.z)) {
						  double max = B.z - it.pos.z+pre;
						  if (max < it.vel.z) {
							it.vel.z = max;
						  }
						}else if ((it.vel.z < 0.0) && (it.pos.z-pre >= B.z+1)){
						  double max = B.z+1 - it.pos.z-pre;
						  if (max > it.vel.z)
							it.vel.z = max;
						}else continue;
						goto zre;
					}
				}
		}
		const double AirResistance = -10.0;
		VectorF resistForce = it.vel * AirResistance * delta;
		resistForce.y = 0.0;
		it.vel = it.vel + resistForce;
    }
    entitiesMutex.unlock();
    DebugInfo::totalSections = sections.size();
}

Entity& World::GetEntity(unsigned int EntityId){
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

std::vector<unsigned int> World::GetEntitiesList() {
    entitiesMutex.lock();
    std::vector<unsigned int> ret;
    for (auto& it : entities) {
        ret.push_back(it.entityId);
    }
    entitiesMutex.unlock();
    return ret;
}

void World::AddEntity(Entity entity) {
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

void World::DeleteEntity(unsigned int EntityId) {
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
    Vector sectionPos(std::floor(pos.x / 16.0),
                      std::floor(pos.y / 16.0),
                      std::floor(pos.z / 16.0));

    Section* section = GetSectionPtr(sectionPos);
    return !section ? BlockId{0, 0} : section->GetBlockId(pos - (sectionPos * 16));
}

void World::SetBlockId(Vector pos, BlockId block) {
    Vector sectionPos(std::floor(pos.x / 16.0),
                      std::floor(pos.y / 16.0),
                      std::floor(pos.z / 16.0));
	Vector blockPos = pos - (sectionPos * 16);
    Section* section = GetSectionPtr(sectionPos);
    section->SetBlockId(blockPos, block);
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

unsigned char World::GetBlockLight(Vector pos)
{
	Vector sectionPos(std::floor(pos.x / 16.0),
		std::floor(pos.y / 16.0),
		std::floor(pos.z / 16.0));

	Vector blockPos = pos - (sectionPos * 16);

	Section* section = GetSectionPtr(sectionPos);
	Section* yp = GetSectionPtr(sectionPos + Vector(0, 1, 0));
	Section* yn = GetSectionPtr(sectionPos + Vector(0, -1, 0));
	Section* xp = GetSectionPtr(sectionPos + Vector(1, 0, 0));
	Section* xn = GetSectionPtr(sectionPos + Vector(-1, 0, 0));
	Section* zp = GetSectionPtr(sectionPos + Vector(0, 0, 1));
	Section* zn = GetSectionPtr(sectionPos + Vector(0, 0, -1));

	if (!section)
		return 0;

	return GetBlockLight(blockPos, section, xp, xn, yp, yn, zp, zn);
}

unsigned char World::GetBlockLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn)
{
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

unsigned char World::GetBlockSkyLight(Vector pos)
{
	Vector sectionPos(	std::floor(pos.x / 16.0),
						std::floor(pos.y / 16.0), 
						std::floor(pos.z / 16.0));

	Vector blockPos = pos - (sectionPos * 16);

	Section* section = GetSectionPtr(sectionPos);
	Section* yp = GetSectionPtr(sectionPos + Vector(0, 1, 0));
	Section* yn = GetSectionPtr(sectionPos + Vector(0, -1, 0));
	Section* xp = GetSectionPtr(sectionPos + Vector(1, 0, 0));
	Section* xn = GetSectionPtr(sectionPos + Vector(-1, 0, 0));
	Section* zp = GetSectionPtr(sectionPos + Vector(0, 0, 1));
	Section* zn = GetSectionPtr(sectionPos + Vector(0, 0, -1));

	if (!section)
		return 0;

	return GetBlockSkyLight(blockPos, section, xp, xn, yp, yn, zp, zn);
}

unsigned char World::GetBlockSkyLight(const Vector &blockPos, const Section *section, const Section *xp, const Section *xn, const Section *yp, const Section *yn, const Section *zp, const Section *zn)
{
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
