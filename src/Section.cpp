#include "Section.hpp"

#include <bitset>

Section::~Section() {
}

Section::Section(Section && other) noexcept {
    using std::swap;
    swap(*this, other);
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

    bool useFirst = false;

    if (useFirst) {
        unsigned char *blocksData = reinterpret_cast<unsigned char*>(data.blocks.data());
        std::vector<unsigned short> blocks;
        blocks.reserve(4096);
        {
            auto begin = std::chrono::steady_clock::now();
            int bitPos = 0;
            unsigned short t = 0;
            for (size_t i = 0; i < data.blocks.size() * 8; i++) {
                for (int j = 0; j < 8; j++) {
                    t |= (blocksData[i] & 0x01) ? 0x80 : 0x00;
                    t >>= 1;
                    blocksData[i] >>= 1;
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
            byte first = t & 0x0F;
            byte second = t >> 4;
            light.push_back(0);
            light.push_back(0);
        }

        std::vector<byte> sky;
        if (!data.sky.empty()) {
            sky.reserve(4096);
            for (int i = 0; i < 2048; i++) {
                byte t = data.sky[i];
                byte first = t & 0x0F;
                byte second = t >> 4;
                sky.push_back(first);
                sky.push_back(second);
            }
        }

        for (int i = 0; i < 4096; i++) {
            unsigned short blockId = !data.palette.empty() ? data.palette[blocks[i]] : blocks[i];
            Block block(blockId >> 4, blockId & 0xF, light[i], sky.empty() ? 0 : sky[i]);
            this->blocks.push_back(block);
        }
    } else {

        std::vector<unsigned short> blocks;
        blocks.reserve(4096);

        unsigned char individualValueMask = ((1 << data.bitsPerBlock) - 1);

        for (int blockNumber = 0; blockNumber < 4096; blockNumber++) {
            int startLong = (blockNumber * data.bitsPerBlock) / 64;
            int startOffset = (blockNumber * data.bitsPerBlock) % 64;
            int endLong = ((blockNumber + 1) * data.bitsPerBlock - 1) / 64;

            unsigned short t;

            if (startLong == endLong) {
                t = (data.blocks[startLong] >> startOffset);
            }
            else {
                int endOffset = 64 - startOffset;
                t = (data.blocks[startLong] >> startOffset | data.blocks[endLong] << endOffset);
            }

            t &= individualValueMask;


            if (t >= data.palette.size()) {
                //LOG(ERROR) << "Out of palette: "<<t;
                blocks.push_back(0);
            }
            else
                blocks.push_back(data.palette.empty() ? t : data.palette[t]);
        }


        std::vector<unsigned char> light;
        light.reserve(4096);
        for (int i = 0; i < 2048; i++) {
            unsigned char t = data.light[i];
            light.push_back(t & 0xF);
            light.push_back(t >> 4 & 0xF);
        }

        std::vector<unsigned char> sky;
        if (!data.sky.empty()) {
            sky.reserve(4096);
            for (int i = 0; i < 2048; i++) {
                unsigned char t = data.sky[i];
                sky.push_back(t & 0xF);
                sky.push_back(t >> 4 & 0xF);
            }
        }

        for (int i = 0; i < 4096; i++) {
            unsigned short blockId = blocks[i];
            Block block(blockId >> 4, blockId & 0xF, light[i], sky.empty() ? 0 : sky[i]);
            this->blocks.push_back(block);
        }
    }
}

Section &Section::operator=(Section other) noexcept {
    using std::swap;
	swap(*this, other);
	return *this;
}

void swap(Section& lhs, Section& rhs) noexcept {
    std::swap(lhs.blocks, rhs.blocks);
    std::swap(lhs.worldPosition, rhs.worldPosition);
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

    for (long long *t = reinterpret_cast<long long *>(dataBlocks); (byte*)t < dataBlocks + dataBlocksLength; t++) {
        long long l = *t;
        endswap(l);
        this->blocks.push_back(l);
    }

    light.assign(dataLight, dataLight + 2048);

    if (dataSky != nullptr) {
        sky.assign(dataSky, dataSky + 2048);
    }
}
