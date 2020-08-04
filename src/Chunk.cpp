#include "Chunk.hpp"
#include "Event.hpp"

#include <easylogging++.h>

#include <bitset>

Chunk::Chunk(Vector2I32 position, bool SkyLight) : pos(position), hasSkyLight(SkyLight) {}

void Chunk::ParseChunk(PacketChunkData *packet) {
	StreamBuffer chunkData(packet->Data.data(), packet->Data.size());
	std::bitset<16> bitmask(static_cast<uint16_t>(packet->PrimaryBitMask));
	for (unsigned char i = 0; i < 16; i++) {
		if (bitmask[i]) {
			std::unique_ptr<Section> *section = &sections[i];
			if (!packet->GroundUpContinuous && !*section)
				LOG(WARNING) << "Chunk updating empty section";

			section->reset(ParseSection(&chunkData, i));
		}
	}
	if (packet->GroundUpContinuous) {
		memcpy(this->biome, packet->Data.data()+(packet->Data.size()-256), 256);
	}
}

static void add(Vector section, std::vector<Vector> *changedForce){
	if (std::find(changedForce->begin(), changedForce->end(), section) == changedForce->end())
		changedForce->push_back(section);
}

void Chunk::UpdateBlock(Vector blockPos, std::vector<Vector> *changedForce) {
	Vector sectionPos(pos.x, blockPos.y / 16, pos.z);

	struct {
		Vector section;
		bool dirty;
	}x, y, z;

	if (blockPos.x == 0)
		x = {sectionPos + Vector(-1, 0, 0), true};
	else if (blockPos.x == 15)
		x = {sectionPos + Vector(1, 0, 0), true};

	if (blockPos.y == 0)
		y = {sectionPos + Vector(0, -1, 0), true};
	else if (blockPos.y == 15)
		y = {sectionPos + Vector(0, 1, 0), true};

	if (blockPos.z == 0)
		z = {sectionPos + Vector(0, 0, -1), true};
	else if (blockPos.z == 15)
		z = {sectionPos + Vector(0, 0, 1), true};

	if(x.dirty)
		add(x.section, changedForce);
	if(y.dirty)
		add(y.section, changedForce);
	if(z.dirty)
		add(z.section, changedForce);
}

void Chunk::ParseChunkData(PacketMultiBlockChange *packet) {
	std::vector<Vector> changedSections, changedForce;
	for (auto& it : packet->Records) {
		int x = (it.HorizontalPosition >> 4 & 15);
		int y = it.YCoordinate;
		int z = (it.HorizontalPosition & 15);

		Vector worldPos(x, y, z);
		Vector sectionPos(pos.x, 0, pos.z);
		floorASRQ(y, 4, sectionPos.y);

		Section *section = sections[sectionPos.y].get();

		if(!section){
			LOG(WARNING) << "Block change empty section";
			continue;
		}

		sections[sectionPos.y]->SetBlockId(worldPos, BlockId{(unsigned short) (it.BlockId >> 4),(unsigned char) (it.BlockId & 0xF) });
		UpdateBlock(Vector(x + (pos.x * 16), y, z + (pos.z * 16)), &changedForce);


		if (std::find(changedSections.begin(), changedSections.end(), sectionPos) == changedSections.end())
			changedSections.push_back(sectionPos);

	}

	for (auto& sectionPos : changedSections)
		PUSH_EVENT("ChunkChanged", sectionPos);

	for (auto& sectionPos : changedForce)
		PUSH_EVENT("ChunkChangedForce", sectionPos);
}

void Chunk::Unload() {
	for (int i = 0; i < 16; i++) {
		if(sections[i])
			PUSH_EVENT("ChunkDeleted", Vector(pos.x, i, pos.z));
	}
}

Section* Chunk::GetSection(unsigned char height) const noexcept {
	return sections[height].get();
}

BlockId Chunk::GetBlockId(Vector blockPos) const noexcept {
	Section* sectionPtr = sections[blockPos.y / 16].get();
	if (!sectionPtr)
		return {0, 0};
	return sectionPtr->GetBlockId(Vector(blockPos.x, blockPos.y % 16, blockPos.z));
}
void Chunk::SetBlockId(Vector blockPos, BlockId block) {
	Vector sectionPos = Vector(pos.x, blockPos.y / 16, pos.z);
	Section* sectionPtr = sections[sectionPos.y].get();
	if (!sectionPtr) {
		LOG(ERROR) << "Updating unloaded section " << sectionPos;
		return;
	}

	sectionPtr->SetBlockId(Vector(blockPos.x, blockPos.y % 16, blockPos.z), block);

	PUSH_EVENT("ChunkChanged", sectionPos);
	if (blockPos.x == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(-1, 0, 0));
	else if (blockPos.x == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(1, 0, 0));
	if (blockPos.y == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, -1, 0));
	else if (blockPos.y == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 1, 0));
	if (blockPos.z == 0)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 0, -1));
	else if (blockPos.z == 15)
		PUSH_EVENT("ChunkChangedForce", sectionPos + Vector(0, 0, 1));
}

Vector2I32 Chunk::GetPosition() const noexcept {
	return pos;
}

Section* Chunk::ParseSection(StreamInput *data, unsigned char height) {
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
	if (hasSkyLight)
		skyLight = data->ReadByteArray(2048);

	long long *blockData = reinterpret_cast<long long*>(dataArray.data());
	for (size_t i = 0; i < dataArray.size() / sizeof(long long); i++)
		endswap(blockData[i]);
	std::vector<long long> blockArray(blockData, blockData + dataArray.size() / sizeof(long long));


	return new Section(
		Vector(this->pos.x, height, this->pos.z), bitsPerBlock, std::move(palette), blockArray,
		std::move(blockLight), std::move(skyLight));
}
