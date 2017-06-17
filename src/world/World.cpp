#include "World.hpp"

void World::ParseChunkData(std::shared_ptr<PacketChunkData> packet) {
	StreamBuffer chunkData(packet->Data.data(), packet->Data.size());
	std::bitset<16> bitmask(packet->PrimaryBitMask);
	for (int i = 0; i < 16; i++) {
		if (bitmask[i]) {
			Vector chunkPosition = Vector(packet->ChunkX, i, packet->ChunkZ);
			Section section = ParseSection(&chunkData);
			auto it = sections.find(chunkPosition);
			if (it == sections.end()) {
				sections.insert(std::make_pair(chunkPosition, section));
			} else {
				using std::swap;
				swap(it->second, section);
			}
			sections.find(chunkPosition)->second.Parse();
		}
	}
}

Section World::ParseSection(StreamInput *data) {
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
	return Section(dataArray.data(), dataArray.size(), blockLight.data(),
	               (skyLight.size() > 0 ? skyLight.data() : nullptr), bitsPerBlock, palette);
}

World::~World() {
}

World::World() {

}

bool World::isPlayerCollides(double X, double Y, double Z) {
	Vector PlayerChunk(floor(X / 16.0), floor(Y / 16.0), floor(Z / 16.0));
	std::vector<Vector> closestSectionsCoordinates = {
			Vector(PlayerChunk.GetX(), PlayerChunk.GetY(), PlayerChunk.GetZ()),
			Vector(PlayerChunk.GetX() + 1, PlayerChunk.GetY(), PlayerChunk.GetZ()),
			Vector(PlayerChunk.GetX() - 1, PlayerChunk.GetY(), PlayerChunk.GetZ()),
			Vector(PlayerChunk.GetX(), PlayerChunk.GetY() + 1, PlayerChunk.GetZ()),
			Vector(PlayerChunk.GetX(), PlayerChunk.GetY() - 1, PlayerChunk.GetZ()),
			Vector(PlayerChunk.GetX(), PlayerChunk.GetY(), PlayerChunk.GetZ() + 1),
			Vector(PlayerChunk.GetX(), PlayerChunk.GetY(), PlayerChunk.GetZ() - 1),
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
		playerColl.x = X - PlayerWidth / 2 - 0.5;
		playerColl.w = PlayerWidth;
		playerColl.y = Y - 0.5f;
		playerColl.h = PlayerHeight;
		playerColl.z = Z - PlayerLength / 2 - 0.5;
		playerColl.l = PlayerLength;

		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				for (int z = 0; z < 16; z++) {
					Block block = it->second.GetBlock(Vector(x, y, z));
					if (block.id == 0 || block.id == 31)
						continue;
					AABB blockColl{(x + it->first.GetX() * 16) - 0.5,
					               (y + it->first.GetY() * 16) - 0.5,
					               (z + it->first.GetZ() * 16) - 0.5, 1, 1, 1};
					if (TestCollision(playerColl, blockColl))
						return true;
				}
			}
		}
	}
	return false;
}
