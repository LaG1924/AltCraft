#include "Block.hpp"

#include <map>
#include <vector>

#include "Plugin.hpp"

static std::vector<BlockInfo> blocks;
static std::map<BlockId, size_t> staticBlockInfo;

BlockInfo WTFBlock{ true, "", "" };

void RegisterStaticBlockInfo(BlockId blockId, BlockInfo blockInfo) {
	//NOTE: It can be made thread-safe using incrementer
	staticBlockInfo[blockId] = blocks.size();
	blocks.push_back(blockInfo);
}

BlockInfo* GetBlockInfo(BlockId blockId, Vector blockPos) {
	auto it = staticBlockInfo.find(blockId);
	if (it != staticBlockInfo.end())
		return &blocks.data()[it->second];
	else
		return &WTFBlock;
}
