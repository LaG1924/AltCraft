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
	case 3: {
		if (blockId.state > 1)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("dirt", "normal"),
			std::pair<std::string,std::string>("coarse_dirt", "normal"),
		};
		return ids[blockId.state];
	}
	case 4: {
		return std::make_pair("cobblestone", "normal");
	}
	case 5: {
		if (blockId.state > 5)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("oak_planks", "normal"),
			std::pair<std::string,std::string>("spruce_planks", "normal"),
			std::pair<std::string,std::string>("birch_panks", "normal"),
			std::pair<std::string,std::string>("jungle_planks", "normal"),
			std::pair<std::string,std::string>("acacia_planks", "normal"),
			std::pair<std::string,std::string>("dark_oak_panks", "normal"),
		};
		return ids[blockId.state];
	}
	case 12: {
		if (blockId.state > 1)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("sand", "normal"),
			std::pair<std::string,std::string>("red_sand", "normal"),
		};
		return ids[blockId.state];
	}
	case 17: {
		unsigned char type = blockId.state & 0x3;
		unsigned char dir = (blockId.state & 0xC) >> 2;
		static const std::string types[] = {
			"oak_log",
			"spruce_log",
			"birch_log",
			"jungle_log",
		};
		static const std::string dirs[] = {
			"axis=y",
			"axis=x",
			"axis=z",
			"axis=none",
		};
		return std::make_pair(types[type], dirs[dir]);
	}
	case 18: {
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("oak_leaves", "normal"),
			std::pair<std::string,std::string>("spruce_leaves", "normal"),
			std::pair<std::string,std::string>("birch_leaves", "normal"),
			std::pair<std::string,std::string>("jungle_leaves", "normal"),
		};
		return ids[blockId.state & 0x3];
	}
	case 31: {
		if (blockId.state > 2)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("dead_bush", "normal"),
			std::pair<std::string,std::string>("tall_grass", "normal"),
			std::pair<std::string,std::string>("fern", "normal"),
		};
		return ids[blockId.state];
	}
	case 35: {
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("white_wool", "normal"),
			std::pair<std::string,std::string>("orange_wool", "normal"),
			std::pair<std::string,std::string>("magenta_wool", "normal"),
			std::pair<std::string,std::string>("light_blue_wool", "normal"),
			std::pair<std::string,std::string>("yellow_wool", "normal"),
			std::pair<std::string,std::string>("lime_wool", "normal"),
			std::pair<std::string,std::string>("pink_wool", "normal"),
			std::pair<std::string,std::string>("gray_wool", "normal"),
			std::pair<std::string,std::string>("light_gray_wool", "normal"),
			std::pair<std::string,std::string>("cyan_wool", "normal"),
			std::pair<std::string,std::string>("purple_wool", "normal"),
			std::pair<std::string,std::string>("blue_wool", "normal"),
			std::pair<std::string,std::string>("green_wool", "normal"),
			std::pair<std::string,std::string>("red_wool", "normal"),
			std::pair<std::string,std::string>("black_wool", "normal"),
		};
		return ids[blockId.state];
	}
	case 37: {
		return std::make_pair("dandelion", "normal");
	}
	case 38: {
		if (blockId.state > 8)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("poppy", "normal"),
			std::pair<std::string,std::string>("blue_orchid", "normal"),
			std::pair<std::string,std::string>("allium", "normal"),
			std::pair<std::string,std::string>("Azure Bluet", "normal"),
			std::pair<std::string,std::string>("red_tulip", "normal"),
			std::pair<std::string,std::string>("orange_tulip", "normal"),
			std::pair<std::string,std::string>("white_tulip", "normal"),
			std::pair<std::string,std::string>("pink_tulip", "normal"),
			std::pair<std::string,std::string>("oxeye_daisy", "normal"),	
		};
		return ids[blockId.state];
	}
	case 175: {
		bool high = ((blockId.state >> 3) & 0x1);
		unsigned char type = blockId.state & 0x7;

		static const std::string types[] = {
			"sunflower",
			"Lilac",
			"double_grass",
			"double_fern",
			"double_rose",
			"Peony",
		};

		static const std::string isHigh[] = {
			"half=lower",
			"half=upper",
		};
		return std::make_pair(types[type], isHigh[high]);
	}
	default:
		break;
	}
	
	return std::make_pair("", "");
}
