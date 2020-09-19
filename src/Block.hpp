#pragma once

#include <utility>
#include <string>

#include "Vector.hpp"
#include "Platform.hpp"

struct BlockId {
	unsigned short id : 12;
    unsigned char state : 4;
};

enum BlockFacing {
    Bottom = 0,
    Top,
    North,
    South,
    West,
    East
};

inline bool operator==(const BlockId& lhs, const BlockId &rhs) {
	return (lhs.id == rhs.id) && (lhs.state == rhs.state);
}

inline bool operator<(const BlockId& lhs, const BlockId &rhs) {
	if (lhs.id != rhs.id)
		return lhs.id < rhs.id;
	return lhs.state < rhs.state;
}

namespace std {
    template <>
    struct hash<BlockId> {
        std::size_t operator()(const BlockId& k) const {
            size_t id = std::hash<unsigned short>()(k.id);
            size_t state = std::hash<unsigned char>()(k.state);

            return (id & state << 1);
        }
    };
}

struct AC_API BlockInfo {
	bool collides;
	std::string blockstate;
	std::string variant;	
};

void RegisterStaticBlockInfo(BlockId blockId, BlockInfo blockInfo);

BlockInfo* GetBlockInfo(BlockId blockId);
