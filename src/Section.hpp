#pragma once

#include <vector>
#include <map>

#include "Block.hpp"
#include "Vector.hpp"

class Section {
    std::vector<long long> block;
	unsigned char light[2048] = {};
	unsigned char sky[2048] = {};
    unsigned char bitsPerBlock = 0;
    std::vector<unsigned short> palette;

	Vector worldPosition;
    mutable size_t hash = 0;

    void CalculateHash() const;

    std::map<Vector, BlockId> overrideList;
public:
    Section(Vector pos, unsigned char bitsPerBlock, std::vector<unsigned short> palette, std::vector<long long> blockData, const std::vector<unsigned char> &lightData, const std::vector<unsigned char> &skyData);
    
	Section() = default;
	
    BlockId GetBlockId(Vector pos) const;
		
    unsigned char GetBlockLight(Vector pos) const;

    unsigned char GetBlockSkyLight(Vector pos) const;

    void SetBlockId(Vector pos, BlockId value);

    void SetBlockLight(Vector pos, unsigned char value);

    void SetBlockSkyLight(Vector pos, unsigned char value);

	Vector GetPosition() const;

    size_t GetHash() const;
};