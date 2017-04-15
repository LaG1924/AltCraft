#pragma once

#include "PositionI.hpp"

class Block {
public:
    Block(int id = 0, unsigned char state = 0, unsigned char light = 15, PositionI position = PositionI());

    Block(unsigned short idAndState, unsigned char light);

    ~Block();

    int GetId();

    int GetState();

    int GetLight();

private:
    int m_id;
    unsigned char m_light;
    PositionI m_position;
    unsigned char m_state;
    //NbtTree* nbt;
};

