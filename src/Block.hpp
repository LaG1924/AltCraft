#pragma once

struct Block {
	Block();

	Block(unsigned short id, unsigned char state, unsigned char light, unsigned char sky);

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