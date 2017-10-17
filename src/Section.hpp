#pragma once

#include <vector>
#include <map>

#include "Block.hpp"
#include "Vector.hpp"

class Section {
    std::vector<long long> block;
    std::vector<unsigned char> light;
    std::vector<unsigned char> sky;
    unsigned char bitsPerBlock;
    std::vector<unsigned short> palette;

	Vector worldPosition;
    mutable size_t hash;

    void CalculateHash() const;

    std::map<Vector, BlockId> overrideList;
public:
    Section(Vector pos, unsigned char bitsPerBlock, std::vector<unsigned short> palette, std::vector<long long> blockData, std::vector<unsigned char> lightData, std::vector<unsigned char> skyData);
    
    Section();

	~Section();

    Section(const Section &other);

    Section(Section &&other) noexcept;

	Section &operator=(Section other) noexcept;

    BlockId GetBlockId(Vector pos) const;

    unsigned char GetBlockLight(Vector pos) const;

    unsigned char GetBlockSkyLight(Vector pos) const;

    void SetBlockId(Vector pos, BlockId value);

    void SetBlockLight(Vector pos, unsigned char value);

    void SetBlockSkyLight(Vector pos, unsigned char value);

	Vector GetPosition() const;

    size_t GetHash() const;

    friend void swap(Section& lhs, Section& rhs) noexcept;
};