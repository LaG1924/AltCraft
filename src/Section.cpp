#include "Section.hpp"

Section::~Section() {
}

Block &Section::GetBlock(Vector pos) {
	return blocks[pos.y * 256 + pos.z * 16 + pos.x];
}

Block Section::GetBlock(Vector pos) const
{
    if (blocks.empty()) {
        static Block fallback;
        return fallback;
    }
    return blocks[pos.y * 256 + pos.z * 16 + pos.x];
}

double totalParsingTime = 0;

Section::Section(PackedSection data)
{
    if (data.blocks.empty())
        return;
    worldPosition = data.position;

    long long *longArray = reinterpret_cast<long long *>(data.blocks.data());
    for (size_t i = 0; i < data.blocks.size() / 8; i++)
        endswap(&longArray[i]);
    std::vector<unsigned short> blocks;
    blocks.reserve(4096);
    {
        auto begin = std::chrono::steady_clock::now();
        int bitPos = 0;
        unsigned short t = 0;
        for (size_t i = 0; i < data.blocks.size(); i++) {
            for (int j = 0; j < 8; j++) {
                t |= (data.blocks.data()[i] & 0x01) ? 0x80 : 0x00;
                t >>= 1;
                data.blocks.data()[i] >>= 1;
                bitPos++;
                if (bitPos >= data.bitsPerBlock) {
                    bitPos = 0;
                    t >>= data.bitsPerBlock - 1;
                    blocks.push_back(t);
                    t = 0;
                }
            }
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> time = end - begin;
        totalParsingTime += time.count();
    }
    std::vector<byte> light;
    light.reserve(4096);
    for (int i = 0; i < 2048; i++) {
        byte t = data.light[i];
        byte first = t & 0xF;
        byte second = (t >> 4) & 0xF;
        light.push_back(0);
        light.push_back(0);
    }

    std::vector<byte> sky;
    if (!data.sky.empty()) {
        sky.reserve(4096);
        for (int i = 0; i < 2048; i++) {
            byte t = data.sky[i];
            byte first = t & 0xF;
            byte second = (t >> 4) & 0xF;
            sky.push_back(first);
            sky.push_back(0xF);
        }
    }


    for (int i = 0; i < 4096; i++) {
        unsigned short blockId = data.palette.size() > 0 ? data.palette[blocks[i]] : blocks[i];
        Block block(blockId >> 4, blockId & 0xF, light[i], sky.empty() ? 0 : sky[i]);
        this->blocks.push_back(block);
    }    
}

Section &Section::operator=(Section other) {
	std::swap(*this, other);
	return *this;
}

void swap(Section &a, Section &b) {
	using std::swap;
	swap(a.blocks, b.blocks);
}

Section::Section(const Section &other) {
	worldPosition = other.worldPosition;	
    this->blocks = other.blocks;
}

Vector Section::GetPosition() const {
	return worldPosition;
}

size_t Section::GetHash() const {
	if (blocks.empty()) return 0;

	const unsigned char *from = reinterpret_cast<const unsigned char *>(blocks.data());
	size_t length = blocks.size() * sizeof(Block);

	std::string str(from, from + length);
	return std::hash<std::string>{}(str);
}

PackedSection::PackedSection(Vector position, byte * dataBlocks, size_t dataBlocksLength, byte * dataLight, byte * dataSky, byte bitsPerBlock, std::vector<unsigned short> palette)
{
    this->position = position;

    this->palette = palette;

    this->bitsPerBlock = bitsPerBlock;

    blocks.assign(dataBlocks, dataBlocks + dataBlocksLength);

    light.assign(dataLight, dataLight + 2048);

    if (dataSky != nullptr) {
        sky.assign(dataSky, dataSky + 2048);
    }
}
