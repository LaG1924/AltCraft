#pragma once

#include <map>
#include "Block.hpp"
#include "Packet.hpp"

const int SECTION_WIDTH=16;
const int SECTION_LENGTH=16;
const int SECTION_HEIGHT=16;

class World {
public:
    Block &GetBlock(PositionI pos);
    void SetBlock(PositionI pos, Block block);
    void ParseChunkData(Packet packet);


    std::map<PositionI,Block> m_blocks;
private:


    size_t ParseSectionData(int chunkX, int chunkZ, bool isGroundContinous, int section, byte *data);

    std::vector<unsigned short>
    ParseBlocks(byte *bytes, int dataLength, std::vector<int> palette, byte bitsPerBlock);

    int m_dimension=0;
};