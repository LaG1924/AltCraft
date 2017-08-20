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

struct PackedSection {
    Vector position;

    int bitsPerBlock;

    std::vector<unsigned short> palette;

    std::vector<unsigned char> blocks;
    std::vector<unsigned char> light;
    std::vector<unsigned char> sky;

    PackedSection(Vector position, byte *dataBlocks, size_t dataBlocksLength, byte *dataLight, byte *dataSky, byte bitsPerBlock,
        std::vector<unsigned short> palette);

    PackedSection() = default;
};

class Section {
	std::vector<Block> blocks;

	Vector worldPosition;

public:
    
    Section(PackedSection data);

	~Section();

	Block &GetBlock(Vector pos);

    Block GetBlock(Vector pos) const;

	Section &operator=(Section other);

	friend void swap(Section &a, Section &b);

	Section(const Section &other);

	Vector GetPosition() const;

    size_t GetHash() const;
};