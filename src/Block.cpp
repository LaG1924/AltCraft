#include "Block.hpp"

#include <map>

#include "Plugin.hpp"

std::map<BlockId, BlockInfo> staticBlockInfo;

void RegisterStaticBlockInfo(BlockId blockId, BlockInfo blockInfo) {
	staticBlockInfo[blockId] = blockInfo;
}

BlockInfo GetBlockInfo(BlockId blockId, Vector blockPos) {
	auto it = staticBlockInfo.find(blockId);
	if (it != staticBlockInfo.end())
		return it->second;
	if (blockPos == Vector())
		return BlockInfo{ true, "", "" };
	return PluginSystem::RequestBlockInfo(blockPos);
}
