#include "Section.hpp"

#include <bitset>

void Section::CalculateHash() const {
    size_t offset = 0;

    std::vector<unsigned char> rawData;    
    rawData.resize(block.size() * sizeof(long long) + light.size() + sky.size());
    
    std::memcpy(rawData.data() + offset, block.data(), block.size() * sizeof(BlockId));
    offset += block.size() * sizeof(BlockId);

    std::memcpy(rawData.data() + offset, light.data(), light.size() * sizeof(unsigned char));
    offset += light.size() * sizeof(unsigned char);

    if (!sky.empty())
        std::memcpy(rawData.data() + offset, sky.data(), sky.size() * sizeof(unsigned char));

    for (auto& it : overrideList) {
        rawData.push_back(*reinterpret_cast<const unsigned short*> (&it.second) & 0xF);
        rawData.push_back(*reinterpret_cast<const unsigned short*> (&it.second) >> 0xF);
    }
    
    const unsigned char *from = reinterpret_cast<const unsigned char *>(rawData.data());
    size_t length = rawData.size();

    std::string str(from, from + length);
    hash =  std::hash<std::string>{}(str);
}

Section::Section(Vector pos, unsigned char bitsPerBlock, std::vector<unsigned short> palette, std::vector<long long> blockData, std::vector<unsigned char> lightData, std::vector<unsigned char> skyData) {
    if (bitsPerBlock < 4)
        bitsPerBlock = 4;
    if (bitsPerBlock > 8)
        bitsPerBlock = 13;
    this->bitsPerBlock = bitsPerBlock;

    this->worldPosition = pos;
    this->block = std::move(blockData);
    this->palette = std::move(palette);
    this->light = std::move(lightData);
    this->sky = std::move(skyData);

    hash = -1;
}

Section::Section():hash(-1),bitsPerBlock(0) {
}

Section::~Section() {

}

Section::Section(Section && other) noexcept {
    using std::swap;
    swap(*this, other);
    hash = -1;
}

Section &Section::operator=(Section other) noexcept {
    using std::swap;
	swap(*this, other);
    hash = -1;
	return *this;
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
}

void swap(Section& lhs, Section& rhs) noexcept {
    std::swap(lhs.block, rhs.block);
    std::swap(lhs.light, rhs.light);
    std::swap(lhs.sky, rhs.sky);
    std::swap(lhs.bitsPerBlock, rhs.bitsPerBlock);
    std::swap(lhs.palette, rhs.palette);
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.worldPosition, rhs.worldPosition);
}

Section::Section(const Section &other) {
	worldPosition = other.worldPosition;	
    this->block = other.block;
    this->light = other.light;
    this->sky = other.sky;
    this->bitsPerBlock = other.bitsPerBlock;
    this->palette = other.palette;
    this->hash = other.hash;
    this->worldPosition = other.worldPosition;
}

Vector Section::GetPosition() const {
	return worldPosition;
}

size_t Section::GetHash() const {
    if (hash == -1)
        CalculateHash();
    return hash;
}