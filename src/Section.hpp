#pragma once

#include <vector>
#include <map>
#include <condition_variable>
#include <functional>

#include <easylogging++.h>

#include "Block.hpp"
#include "Vector.hpp"
#include "Utility.hpp"

const int SECTION_WIDTH = 16;
const int SECTION_LENGTH = 16;
const int SECTION_HEIGHT = 16;

class Section {
	std::vector<unsigned short> m_palette;
	byte *m_dataBlocks = nullptr;
	size_t m_dataBlocksLen;
	byte *m_dataLight = nullptr;
	byte *m_dataSkyLight = nullptr;
	byte m_bitsPerBlock = 0;
	std::vector<Block> m_blocks;
	std::condition_variable parseWaiter;

	Section();

	Vector worldPosition;

public:
	void Parse();

	Section(Vector position, byte *dataBlocks, size_t dataBlocksLength, byte *dataLight, byte *dataSky, byte bitsPerBlock,
	        std::vector<unsigned short> palette);

	~Section();

	Block &GetBlock(Vector pos);

	Section &operator=(Section other);

	friend void swap(Section &a, Section &b);

	Section(const Section &other);

	Vector GetPosition();

    size_t GetHash();
};