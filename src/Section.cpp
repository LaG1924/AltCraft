#include "Section.hpp"

#include <bitset>
#include <cstring>

void Section::CalculateHash() const {
    if (block.empty()) {
        hash = 0;
        return;
    }


    size_t offset = 0;

    std::vector<unsigned char> rawData;    
    rawData.resize(block.size() * sizeof(long long) + 4096);
    
	std::memcpy(rawData.data(), light, 2048);
	std::memcpy(rawData.data() + 2048, sky, 2048);
	std::memcpy(rawData.data() + 4096, block.data(), block.size() * sizeof(long long));    
	
    for (auto& it : overrideList) {
        rawData.push_back(*reinterpret_cast<const unsigned short*> (&it.second) & 0xF);
        rawData.push_back(*reinterpret_cast<const unsigned short*> (&it.second) >> 0xF);
    }
    
    const unsigned char *from = reinterpret_cast<const unsigned char *>(rawData.data());
    size_t length = rawData.size();

    std::string str(from, from + length);
    hash =  std::hash<std::string>{}(str);
}

Section::Section(Vector pos, unsigned char bitsPerBlock, std::vector<unsigned short> palette, std::vector<long long> blockData, const std::vector<unsigned char> &lightData, const std::vector<unsigned char> &skyData) {
    if (bitsPerBlock < 4)
        bitsPerBlock = 4;
    if (bitsPerBlock > 8)
        bitsPerBlock = 13;
    this->bitsPerBlock = bitsPerBlock;

    this->worldPosition = pos;
    this->block = std::move(blockData);
    this->palette = std::move(palette);
	std::copy(lightData.begin(), lightData.end(), light);
	std::copy(skyData.begin(), skyData.end(), sky);

    hash = -1;
	CalculateHash();
}

BlockId Section::GetBlockId(Vector pos) const {
    if (block.empty())
        return BlockId{ 0,0 };

    if (!overrideList.empty()) {
        auto iter = overrideList.find(pos);
        if (iter != overrideList.end())
            return iter->second;
    }

    int value;

    unsigned char individualValueMask = ((1 << bitsPerBlock) - 1);

    int blockNumber = (((pos.y * 16) + pos.z) * 16) + pos.x;
    int startLong = (blockNumber * bitsPerBlock) / 64;
    int startOffset = (blockNumber * bitsPerBlock) % 64;
    int endLong = ((blockNumber + 1) * bitsPerBlock - 1) / 64;

    unsigned char t;

    if (startLong == endLong) {
        t = (block[startLong] >> startOffset);
    }
    else {
        int endOffset = 64 - startOffset;
        t = (block[startLong] >> startOffset |block[endLong] << endOffset);
    }

    t &= individualValueMask;


    if (t >= palette.size()) {
        //LOG(ERROR) << "Out of palette: " << t;
        value = 0;
    }
    else
        value = palette[t];

    BlockId blockId;
    blockId.id = value >> 4;
    blockId.state = value & 0xF;
    return blockId;
}

unsigned char Section::GetBlockLight(Vector pos) const
{
	int blockNumber = pos.y * 256 + pos.z * 16 + pos.x;
	unsigned char lightValue = this->light[blockNumber / 2];
	return (blockNumber % 2 == 0) ? (lightValue & 0xF) : (lightValue >> 4);
}

unsigned char Section::GetBlockSkyLight(Vector pos) const
{
	int blockNumber = pos.y * 256 + pos.z * 16 + pos.x;
    unsigned char skyValue = this->sky[blockNumber / 2];
    return (blockNumber % 2 == 0) ? (skyValue & 0xF) : (skyValue >> 4);
}

void Section::SetBlockId(Vector pos, BlockId value) {
    overrideList[pos] = value;
    hash = -1;
	CalculateHash();
}

Vector Section::GetPosition() const {
	return worldPosition;
}

size_t Section::GetHash() const {
    if (hash == -1)
        CalculateHash();
    return hash;
}