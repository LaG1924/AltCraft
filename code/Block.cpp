#include "Block.hpp"

Block::~Block() {}

Block::Block(unsigned short idAndState, unsigned char light) : id(idAndState >> 4), state(idAndState & 0x0F),
                                                               light(light) {}

Block::Block(unsigned short id, unsigned char state, unsigned char light) : id(id), state(state), light(light) {}

Block::Block() : id(0), state(0), light(0) {}
