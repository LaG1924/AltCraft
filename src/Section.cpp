#include "Section.hpp"

Section::~Section() {
}

Block &Section::GetBlock(Vector pos) {
	return blocks[pos.y * 256 + pos.z * 16 + pos.x];
}

Block Section::GetBlock(Vector pos) const
{
    return blocks[pos.y * 256 + pos.z * 16 + pos.x];
}

double totalParsingTime = 0;

//void Section::Parse() {
//	if (!m_blocks.empty())
//		return;
//
//	long long *longArray = reinterpret_cast<long long *>(m_dataBlocks);
//	for (size_t i = 0; i < m_dataBlocksLen / 8; i++)
//		endswap(&longArray[i]);
//	std::vector<unsigned short> blocks;
//	blocks.reserve(4096);
//	{
//		auto begin = std::chrono::steady_clock::now();
//		int bitPos = 0;
//		unsigned short t = 0;
//		for (size_t i = 0; i < m_dataBlocksLen; i++) {
//			for (int j = 0; j < 8; j++) {
//				t |= (m_dataBlocks[i] & 0x01) ? 0x80 : 0x00;
//				t >>= 1;
//				m_dataBlocks[i] >>= 1;
//				bitPos++;
//				if (bitPos >= m_bitsPerBlock) {
//					bitPos = 0;
//					t >>= m_bitsPerBlock - 1;
//					blocks.push_back(t);
//					t = 0;
//				}
//			}
//		}
//		auto end = std::chrono::steady_clock::now();
//		std::chrono::duration<double, std::milli> time = end - begin;
//		totalParsingTime += time.count();
//	}
//	std::vector<byte> light;
//	light.reserve(4096);
//	for (int i = 0; i < 2048; i++) {
//		byte t = m_dataLight[i];
//		byte first = t & 0b11110000;
//		byte second = t >> 4;
//		light.push_back(first);
//		light.push_back(second);
//	}
//	for (int i = 0; i < 4096; i++) {
//		unsigned short blockId = m_palette.size() > 0 ? m_palette[blocks[i]] : blocks[i];
//		Block block(blockId >> 4, blockId & 0xF);
//		m_blocks.push_back(block);
//	}
//	delete[] m_dataBlocks;
//	m_dataBlocksLen = 0;
//	m_dataBlocks = nullptr;
//	delete[] m_dataLight;
//	m_dataLight = nullptr;
//	delete[] m_dataSkyLight;
//	m_dataSkyLight = nullptr;
//
//	parseWaiter.notify_all();
//}

Section::Section(PackedSection data)
{
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
        byte first = t & 0b11110000;
        byte second = t >> 4;
        light.push_back(first);
        light.push_back(second);
    }
    for (int i = 0; i < 4096; i++) {
        unsigned short blockId = data.palette.size() > 0 ? data.palette[blocks[i]] : blocks[i];
        Block block(blockId >> 4, blockId & 0xF);
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
	//std::copy(other.blocks.begin(), other.blocks.end(), std::back_inserter(blocks));
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
