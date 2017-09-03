#include "Section.hpp"

#include <bitset>

void Section::CalculateHash() {
    std::vector<unsigned char> rawData;
    rawData.reserve(block.size() * sizeof(long long) + light.size() + sky.size());
    std::copy(block.begin(), block.end(), std::back_inserter(rawData));
    std::copy(light.begin(), light.end(), std::back_inserter(rawData));
    if (!sky.empty())
        std::copy(sky.begin(), sky.end(), std::back_inserter(rawData));

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

    CalculateHash();
}

Section::Section() {

    CalculateHash();    
}

Section::~Section() {

}

Section::Section(Section && other) noexcept {
    using std::swap;
    swap(*this, other);
    CalculateHash();
}

Section &Section::operator=(Section other) noexcept {
    using std::swap;
	swap(*this, other);
    CalculateHash();
	return *this;
}

BlockId Section::GetBlockId(Vector pos) const {
    if (block.empty())
        return BlockId{ 0,0 };
    int value;

    unsigned char individualValueMask = ((1 << bitsPerBlock) - 1);

    int blockNumber = (((pos.y * 16) + pos.z) * 16) + pos.x;
    int startLong = (blockNumber * bitsPerBlock) / 64;
    int startOffset = (blockNumber * bitsPerBlock) % 64;
    int endLong = ((blockNumber + 1) * bitsPerBlock - 1) / 64;

    unsigned short t;

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
    LOG(WARNING) << "Block changing not implemented!";
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
    return hash;
}