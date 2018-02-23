#pragma once

#include <functional>

struct Block {
    Block();
    Block(unsigned short id, unsigned char state,
          unsigned char light, unsigned char sky);
    ~Block();

    unsigned short id : 13;
    unsigned char state : 4;
    unsigned char light : 4;
    unsigned char sky : 4;
};

struct BlockId {
    unsigned short id : 13;
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

bool operator==(const BlockId& lhs, const BlockId &rhs);

bool operator<(const BlockId& lhs, const BlockId &rhs);

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