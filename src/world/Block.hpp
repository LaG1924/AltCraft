#pragma once

struct Block {
    Block();

    Block(unsigned short id, unsigned short state);

    ~Block();

    unsigned short id:13;
    unsigned char state:4;
    //unsigned char light:4;
};

bool operator<(const Block &lhs, const Block &rhs);