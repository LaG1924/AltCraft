#include "Block.hpp"

Block::~Block() {}

Block::Block(unsigned short idAndState, unsigned char light) : id(idAndState >> 4), state(idAndState & 0x0F) {}

Block::Block(unsigned short id, unsigned char state, unsigned char light) : id(id), state(state) {}

Block::Block() : id(0), state(0) {}

bool operator<(const Block &lhs, const Block &rhs) {
    if (lhs.id < rhs.id)
        return true;
    if (lhs.id == rhs.id) {
        if (lhs.state != rhs.state)
            return lhs.state < rhs.state;
    }
    return false;
}
