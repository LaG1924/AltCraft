#include "Block.hpp"

#include <string>

Block::~Block() {}

Block::Block(unsigned short id, unsigned char state,
	         unsigned char light, unsigned char sky)
        : id(id), state(state), light(light), sky (sky) {}

Block::Block() : id(0), state(0), light(0), sky(0) {}

bool operator==(const BlockId& lhs, const BlockId &rhs) {
    return (lhs.id == rhs.id) && (lhs.state == rhs.state);
}

bool operator<(const BlockId& lhs, const BlockId &rhs) {
	if (lhs.id < rhs.id)
		return true;
    return lhs.state < rhs.state;
}

std::pair<std::string, std::string> TransformBlockIdToBlockStateName(BlockId blockId) {
	switch (blockId.id) {
	case 1: {
		if (blockId.state > 6)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("stone", "normal"),
			std::pair<std::string,std::string>("granite", "normal"),
			std::pair<std::string,std::string>("smooth_granite", "normal"),
			std::pair<std::string,std::string>("diorite", "normal"),
			std::pair<std::string,std::string>("smooth_diorite", "normal"),
			std::pair<std::string,std::string>("andesite", "normal"),
			std::pair<std::string,std::string>("smooth_andesite", "normal"),
		};
		return ids[blockId.state];		
	}
	case 2: {
		return std::make_pair("grass", "snowy=false");
	}

	default:
		break;
	}
	
	return std::make_pair("", "");
}
