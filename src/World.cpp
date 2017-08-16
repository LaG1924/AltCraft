#include "World.hpp"
#include "Event.hpp"

void World::ParseChunkData(std::shared_ptr<PacketChunkData> packet) {
	StreamBuffer chunkData(packet->Data.data(), packet->Data.size());    
	std::bitset<16> bitmask(packet->PrimaryBitMask);
	for (int i = 0; i < 16; i++) {
		if (bitmask[i]) {
			Vector chunkPosition = Vector(packet->ChunkX, i, packet->ChunkZ);
			Section section = ParseSection(&chunkData, chunkPosition);
            parseMutex.lock();            
            toParse.push(section);
            parseMutex.unlock();
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

Section World::ParseSection(StreamInput *data, Vector position) {
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
	return Section(position, dataArray.data(), dataArray.size(), blockLight.data(),
	               (skyLight.size() > 0 ? skyLight.data() : nullptr), bitsPerBlock, palette);
}

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

World::~World() {
    isRunning = false;
    parser.join();
}

World::World() {
    parser = std::thread(&World::ParserFunc, this);
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
	std::vector<std::map<Vector, Section>::iterator> closestSections;
	for (auto &coord:closestSectionsCoordinates) {
		auto it = sections.find(coord);
		if (it != sections.end())
			closestSections.push_back(it);
	}
	if (closestSections.empty())
		return false;

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

		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				for (int z = 0; z < 16; z++) {
					Block block = it->second.GetBlock(Vector(x, y, z));
					if (block.id == 0 || block.id == 31)
						continue;
					AABB blockColl{(x + it->first.x * 16.0),
					               (y + it->first.y * 16.0),
					               (z + it->first.z * 16.0), 1, 1, 1};
					if (TestCollision(playerColl, blockColl))
						return true;
				}
			}
		}
	}
	return false;
}

Block &World::GetBlock(Vector pos) {
	Vector sectionPos (floor(pos.x / 16.0f),floor(pos.y / 16.0f),floor(pos.z/16.0f));
	Vector inSectionPos = pos - (sectionPos * 16u);
	if (sections.find(sectionPos)==sections.end()){
		static Block block(0,0);
		return block;
	}
	sectionMutexes[sectionPos].lock();
	Block& block = sections.find(sectionPos)->second.GetBlock(inSectionPos);
	sectionMutexes[sectionPos].unlock();
	return block;
}

std::vector<Vector> World::GetSectionsList() {
	std::vector<Vector> sectionsList;
	for (auto& it:sections) {
		sectionsList.push_back(it.first);
	}
	return sectionsList;
}

Section &World::GetSection(Vector sectionPos) {
	sectionMutexes[sectionPos].lock();
	sectionMutexes[sectionPos].unlock();
	return sections.find(sectionPos)->second;
}

glm::vec3 World::Raycast(glm::vec3 position, glm::vec3 direction, float maxLength, float minPrecision) {
    return glm::vec3(position * direction / maxLength * minPrecision);
}

void World::UpdatePhysics(float delta)
{
    delta /= 5;
    for (auto& it : entities) {
        it.pos = it.pos + it.vel * delta;
    }
}

Entity & World::GetEntity(unsigned int EntityId)
{
    for (auto& it : entities) {
        if (it.entityId == EntityId) {
            return it;
        }
    }
    static Entity fallback;
    return fallback;
}

std::vector<unsigned int> World::GetEntitiesList()
{
    std::vector<unsigned int> ret;
    for (auto& it : entities) {
        ret.push_back(it.entityId);
    }
    return ret;
}

void World::AddEntity(Entity entity)
{
    for (auto& it : entities) {
        if (it.entityId == entity.entityId) {
            LOG(ERROR) << "Adding already existing entity" << entity.entityId;
            return;
        }
    }
    entities.push_back(entity);

}
