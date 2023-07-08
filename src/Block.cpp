#include "Block.hpp"

#include <map>

static std::map<BlockId, BlockInfo> blocks;
static std::map<BlockId, LiquidInfo> liquids;

static BlockInfo UnknownBlock{ true, "", "" };
static LiquidInfo UnknownLiquid{ "", "" };

void RegisterStaticBlockInfo(BlockId blockId, BlockInfo blockInfo) {
    blocks.try_emplace(blockId, blockInfo);
}

void RegisterStaticLiquidInfo(BlockId blockId, const LiquidInfo& liquidInfo) {
    liquids[blockId] = liquidInfo;
    for (uint8_t i = 0; i < 16; i++)
        blocks.try_emplace(BlockId{ blockId.id, i }, BlockInfo{ true, "@liquid", liquidInfo.stillTexture });
}

BlockInfo* GetBlockInfo(BlockId blockId) {
    auto it = blocks.find(blockId);
    return it != blocks.end() ? &it->second : &UnknownBlock;
}

const LiquidInfo& GetBlockLiquidInfo(BlockId blockId) {
    auto it = liquids.find(blockId);
    return it != liquids.end() ? it->second : UnknownLiquid;
}
