#pragma once

#include <vector>
#include <map>
#include <condition_variable>
#include <functional>

#include <easylogging++.h>

#include "Block.hpp"
#include "Vector.hpp"
#include "Utility.hpp"

class Section {
    std::vector<long long> block;
    std::vector<unsigned char> light;
    std::vector<unsigned char> sky;
    unsigned char bitsPerBlock;
    std::vector<unsigned short> palette;

	Vector worldPosition;
    size_t hash;

    void CalculateHash();
public:
    Section(Vector pos, unsigned char bitsPerBlock, std::vector<unsigned short> palette, std::vector<long long> blockData, std::vector<unsigned char> lightData, std::vector<unsigned char> skyData);
    
    Section();

	~Section();

    Section(const Section &other);

    Section(Section &&other) noexcept;

	Section &operator=(Section other) noexcept;

    BlockId GetBlockId(Vector pos) const;

    unsigned char GetBlockLight(Vector pos);

    unsigned char GetBlockSkyLight(Vector pos);

    void SetBlockId(Vector pos, BlockId value);

    void SetBlockLight(Vector pos, unsigned char value);

    void SetBlockSkyLight(Vector pos, unsigned char value);

	Vector GetPosition() const;

    size_t GetHash() const;

    friend void swap(Section& lhs, Section& rhs) noexcept;
};