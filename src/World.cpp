#include "World.hpp"
#include "Event.hpp"

void World::ParseChunkData(std::shared_ptr<PacketChunkData> packet) {
	StreamBuffer chunkData(packet->Data.data(), packet->Data.size());    
	std::bitset<16> bitmask(packet->PrimaryBitMask);
	for (int i = 0; i < 16; i++) {
		if (bitmask[i]) {
			Vector chunkPosition = Vector(packet->ChunkX, i, packet->ChunkZ);
			PackedSection section = ParseSection(&chunkData, chunkPosition);
            auto it = sections.find(chunkPosition);
            if (it == sections.end()) {
                sections.insert(std::make_pair(chunkPosition, section));
            }
            else {
                using std::swap;
                swap(it->second, section);
            }
            EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ chunkPosition });

            /*parseMutex.lock();            
            toParse.push(section);
            parseMutex.unlock();*/
			/*section.Parse();
			sectionMutexes[chunkPosition].lock();
			auto it = sections.find(chunkPosition);
			if (it == sections.end()) {
				sections.insert(std::make_pair(chunkPosition, section));
			} else {
				using std::swap;
				swap(it->second, section);
			}
			sectionMutexes[chunkPosition].unlock();

            EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ chunkPosition });*/
		}
	}
}

PackedSection World::ParseSection(StreamInput *data, Vector position) {
	unsigned char bitsPerBlock = data->ReadUByte();
	int paletteLength = data->ReadVarInt();
	std::vector<unsigned short> palette;
	for (int i = 0; i < paletteLength; i++) {
		palette.push_back(data->ReadVarInt());
	}
	int dataArrayLength = data->ReadVarInt();
	auto dataArray = data->ReadByteArray(dataArrayLength * 8);
	auto blockLight = data->ReadByteArray(4096 / 2);
	std::vector<unsigned char> skyLight;
	if (dimension == 0)
		skyLight = data->ReadByteArray(4096 / 2);
	return PackedSection(position, dataArray.data(), dataArray.size(), blockLight.data(),
	               (skyLight.size() > 0 ? skyLight.data() : nullptr), bitsPerBlock, palette);
}
/*
void World::ParserFunc()
{
    LoopExecutionTimeController timer(std::chrono::milliseconds(32));
    while (isRunning) {
        parseMutex.lock();
        while (toParse.size() > 0) {
            Section section = toParse.front();
            toParse.pop();
            parseMutex.unlock();

            section.Parse();
            sectionMutexes[section.GetPosition()].lock();
            auto it = sections.find(section.GetPosition());
            if (it == sections.end()) {
                sections.insert(std::make_pair(section.GetPosition(), section));
            }
            else {
                using std::swap;
                swap(it->second, section);
            }
            sectionMutexes[section.GetPosition()].unlock();

            EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ section.GetPosition() });

            parseMutex.lock();
        }
        parseMutex.unlock();
        timer.Update();
    }
}
*/
World::~World() {
}

Block & World::GetBlock(Vector worldPosition)
{
    Vector sectionPos(std::floor(worldPosition.x / 16.0), std::floor(worldPosition.y / 16.0), std::floor(worldPosition.z / 16.0));
    auto sectionIt = sections.find(sectionPos);
    if (sectionIt != sections.end()) {
        Section section(sectionIt->second);
        auto result = cachedSections.insert(std::make_pair(sectionPos, section));
        sections.erase(sectionIt);
    }
    auto it = cachedSections.find(sectionPos);
    if (it == cachedSections.end()) {
        static Block fallbackBlock;
        return fallbackBlock;
    }
    Section& section = it->second;
    Block& block = section.GetBlock(worldPosition - sectionPos * 16);
    return block;
}

World::World() {
}

bool World::isPlayerCollides(double X, double Y, double Z) {
	Vector PlayerChunk(floor(X / 16.0), floor(Y / 16.0), floor(Z / 16.0));
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
	for (auto &coord:closestSectionsCoordinates) {
        if (sections.find(coord) != sections.end())
            closestSections.push_back(coord);
        else if (cachedSections.find(coord) != cachedSections.end())
            closestSections.push_back(coord);
	}

	for (auto &it:closestSections) {

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
					Block block = section.GetBlock(Vector(x, y, z));
					if (block.id == 0 || block.id == 31)
						continue;
					AABB blockColl{(x + it.x * 16.0),
					               (y + it.y * 16.0),
					               (z + it.z * 16.0), 1, 1, 1};
					if (TestCollision(playerColl, blockColl))
						return true;
				}
			}
		}
	}
	return false;
}

std::vector<Vector> World::GetSectionsList() {
	std::vector<Vector> sectionsList;
	for (auto& it:sections) {
		sectionsList.push_back(it.first);
	}
    for (auto& it : cachedSections) {
        if (std::find(sectionsList.begin(), sectionsList.end(), it.first) == sectionsList.end())
            sectionsList.push_back(it.first);
    }
	return sectionsList;
}

const Section &World::GetSection(Vector sectionPos) {
    auto result = cachedSections.find(sectionPos);
    if (result == cachedSections.end()) {
        auto it = sections.find(sectionPos);
        if (it == sections.end())
            LOG(ERROR) << "BAD";
        Section section(it->second);
        auto result = cachedSections.insert(std::make_pair(sectionPos, section));
        return result.first->second;
    } else {
        return result->second;
    }
}

glm::vec3 World::Raycast(glm::vec3 position, glm::vec3 direction, float maxLength, float minPrecision) {
    return glm::vec3(position * direction / maxLength * minPrecision);
}

void World::UpdatePhysics(float delta)
{
    //delta /= 5;
    entitiesMutex.lock();
    for (auto& it : entities) {
        it.pos = it.pos + it.vel * delta;
    }
    entitiesMutex.unlock();
}
//Faces: 14 650 653
//Models: 937.641.792 Bytes x64
//Textures: 234.410.448 Bytes x16
//Colors: 175.807.836 Bytes x12

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
            LOG(ERROR) << "Adding already existing entity" << entity.entityId;
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
    Block& block = this->GetBlock(packet->Position);
    block = Block(packet->BlockId >> 4, packet->BlockId & 15);
    Vector sectionPos(std::floor(packet->Position.x / 16.0), std::floor(packet->Position.y / 16.0), std::floor(packet->Position.z / 16.0));
    EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ sectionPos });
}

void World::ParseChunkData(std::shared_ptr<PacketMultiBlockChange> packet) {
    std::vector<Vector> changedSections;
    for (auto& it : packet->Records) {
        int x = (it.HorizontalPosition >> 4 & 15) + (packet->ChunkX * 16);
        int y = it.YCoordinate;
        int z = (it.HorizontalPosition & 15) + (packet->ChunkZ * 16);
        Vector worldPos(x, y, z);
        Block& block = GetBlock(worldPos);
        block = Block(it.BlockId >> 4, it.BlockId & 15);

        Vector sectionPos(packet->ChunkX, std::floor(it.YCoordinate / 16.0), packet->ChunkZ);
        if (std::find(changedSections.begin(), changedSections.end(), sectionPos) == changedSections.end())
            changedSections.push_back(sectionPos);
    }
    for (auto& sectionPos: changedSections)
        EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ sectionPos });
}