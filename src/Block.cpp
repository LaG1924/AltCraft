#include "Block.hpp"

#include <map>

#include "Plugin.hpp"

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
		if (blockId.state > 2)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("dirt", "normal"),
			std::pair<std::string,std::string>("coarse_dirt", "normal"),
			std::pair<std::string,std::string>("podzol", "snowy=false"),
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
			std::pair<std::string,std::string>("birch_planks", "normal"),
			std::pair<std::string,std::string>("jungle_planks", "normal"),
			std::pair<std::string,std::string>("acacia_planks", "normal"),
			std::pair<std::string,std::string>("dark_oak_planks", "normal"),
		};
		return ids[blockId.state];
	}
	case 7: {
		return std::make_pair("bedrock", "normal");
	}
	case 8:
	case 9: {
		return std::make_pair("water", "normal");
	}
	case 10:
	case 11: {
		return std::make_pair("lava", "normal");
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
	case 13: {
		return std::make_pair("gravel", "normal");
	}
	case 14: {
		return std::make_pair("gold_ore", "normal");
	}
	case 15: {
		return std::make_pair("iron_ore", "normal");
	}
	case 16: {
		return std::make_pair("coal_ore", "normal");
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
	case 20: {
		return std::make_pair("glass", "normal");
	}
	case 21: {
		return std::make_pair("lapis_ore", "normal");
	}
	case 22: {
		return std::make_pair("lapis_block", "normal");
	}
	case 24: {
		if (blockId.state > 2)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("sandstone", "normal"),
			std::pair<std::string,std::string>("chiseled_sandstone", "normal"),
			std::pair<std::string,std::string>("smooth_sandstone", "normal"),
		};
		return ids[blockId.state];
	}
	case 30: {
		return std::make_pair("web", "normal");
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
	case 32: {
		return std::make_pair("dead_bush", "normal");
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
			std::pair<std::string,std::string>("silver_wool", "normal"),
			std::pair<std::string,std::string>("cyan_wool", "normal"),
			std::pair<std::string,std::string>("purple_wool", "normal"),
			std::pair<std::string,std::string>("blue_wool", "normal"),
			std::pair<std::string,std::string>("brown_wool", "normal"),
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
			std::pair<std::string,std::string>("houstonia", "normal"),
			std::pair<std::string,std::string>("red_tulip", "normal"),
			std::pair<std::string,std::string>("orange_tulip", "normal"),
			std::pair<std::string,std::string>("white_tulip", "normal"),
			std::pair<std::string,std::string>("pink_tulip", "normal"),
			std::pair<std::string,std::string>("oxeye_daisy", "normal"),	
		};
		return ids[blockId.state];
	}
	case 39: {
		return std::make_pair("brown_mushroom","normal");
	}
	case 40: {
		return std::make_pair("red_mushroom", "normal");
	}
	case 41: {
		return std::make_pair("gold_block", "normal");
	}
	case 44: {
		return std::make_pair("stone_slab", "half=" + std::string(!(blockId.state >> 3) ? "bottom" : "top"));
	}
	case 45: {
		return std::make_pair("brick_block", "normal");
	}
	case 46: {
		return std::make_pair("tnt", "normal");
	}
	case 47: {
		return std::make_pair("bookshelf", "normal");
	}
	case 48: {
		return std::make_pair("mossy_cobblestone", "normal");
	}
	case 49: {
		return std::make_pair("obsidian", "normal");
	}
	case 50: {
		if (blockId.state > 5)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("", ""),
			std::pair<std::string,std::string>("torch", "facing=east"),
			std::pair<std::string,std::string>("torch", "facing=west"),
			std::pair<std::string,std::string>("torch", "facing=south"),
			std::pair<std::string,std::string>("torch", "facing=north"),
			std::pair<std::string,std::string>("torch", "facing=up"),
		};
		return ids[blockId.state];
	}
	case 53: {
		bool isUp = blockId.state >> 2;
		unsigned char dir = blockId.state & 0x3;
		static const std::string dirs[] = {
			"facing=east,half=",
			"facing=west,half=",
			"facing=south,half=",
			"facing=north,half=",			
		};

		return std::make_pair("oak_stairs", dirs[dir] + (isUp? "top" : "bottom") +",shape=straight");
	}
	case 56: {
		return std::make_pair("diamond_ore", "normal");
	}
	case 57: {
		return std::make_pair("diamond_block", "normal");
	}
	case 59: {
		return std::make_pair("wheat", "age=" + std::to_string(blockId.state));
	}
	case 60: {
		return std::make_pair("farmland", "moisture=" + std::to_string(7 - blockId.state));
	}
	case 61: {
		static const std::string dirs[] = {
			"",
			"",
			"facing=north",
			"facing=south",
			"facing=west",
			"facing=east",
		};
		return std::make_pair("furnace", dirs[blockId.state]);
	}
	case 64: {
		bool isUp = !(blockId.state >> 3);
		bool hingeIsLeft = true;
		bool isOpen = 0;
		unsigned char dir = 0;
		if (isUp)
			hingeIsLeft = blockId.state & 0x1;
		else {
			isOpen = (blockId.state >> 1) & 0x1;
			dir = blockId.state >> 2;
		}
		static const std::string dirs[] = {
			"east",
			"south",
			"west",
			"north",
		};

		return std::make_pair("wooden_door", "facing=" + dirs[dir] + ",half=" + (isUp ? "upper" : "lower") + ",hinge=" + (hingeIsLeft ? "left" : "right") + ",open=" + (isOpen ? "true" : "false"));
	}
	case 67: {
		bool isUp = blockId.state >> 2;
		unsigned char dir = blockId.state & 0x3;
		static const std::string dirs[] = {
			"facing=east,half=",
			"facing=west,half=",
			"facing=south,half=",
			"facing=north,half=",
		};

		return std::make_pair("stone_stairs", dirs[dir] + (isUp ? "top" : "bottom") + ",shape=straight");
	}
	case 69: {
		bool isActive = blockId.state >> 3;
		static const std::string types[] = {
			"facing=down_x",
			"facing=east",
			"facing=west",
			"facing=south",
			"facing=north",
			"facing=up_z",
			"facing=up_x",
			"facing=down_z",
		};
		
		return std::make_pair("lever", types[blockId.state & 0x7] + ",powered=" + (isActive ? "true" : "false"));
	}
	case 73: {
		return std::make_pair("redstone_ore", "normal");
	}
	case 74: {
		return std::make_pair("redstone_ore", "normal");
	}
	case 78: {
		if (blockId.state > 7)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("snow_layer", "layers=1"),
			std::pair<std::string,std::string>("snow_layer", "layers=2"),
			std::pair<std::string,std::string>("snow_layer", "layers=3"),
			std::pair<std::string,std::string>("snow_layer", "layers=4"),
			std::pair<std::string,std::string>("snow_layer", "layers=5"),
			std::pair<std::string,std::string>("snow_layer", "layers=6"),
			std::pair<std::string,std::string>("snow_layer", "layers=7"),
			std::pair<std::string,std::string>("snow_layer", "layers=8"),
		};
		return ids[blockId.state];
	}
	case 79: {
		return std::make_pair("ice", "normal");
	}
	case 80: {
		return std::make_pair("snow", "normal");
	}
	case 81: {
		return std::make_pair("cactus", "normal");
	}
	case 82: {
		return std::make_pair("clay", "normal");
	}
	case 83: {
		return std::make_pair("reeds", "normal");
	}
	case 86: {
		if (blockId.state > 3)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("pumpkin", "facing=south"),
			std::pair<std::string,std::string>("pumpkin", "facing=west"),
			std::pair<std::string,std::string>("pumpkin", "facing=north"),
			std::pair<std::string,std::string>("pumpkin", "facing=east"),
		};
		return ids[blockId.state];
	}
	case 87: {
		return std::make_pair("netherrack", "normal");
	}
	case 88: {
		return std::make_pair("soul_sand", "normal");
	}
	case 89: {
		return std::make_pair("glowstone", "normal");
	}
	case 90: {
		return std::make_pair("portal", blockId.state == 1 ? "axis=x" : "axis=z");
	}
	case 93: {
		static const std::string dirs[] = {
			"east",
			"south",
			"west",
			"north",
		};
		unsigned char dir = blockId.state & 0x3;
		unsigned char delay = (blockId.state >> 2) + 1;
		return std::make_pair("unpowered_repeater", "delay=" + std::to_string(delay) + ",facing=" + dirs[dir] + ",locked=false");
	}
	case 94: {
		static const std::string dirs[] = {
			"east",
			"south",
			"west",
			"north",
		};
		unsigned char dir = blockId.state & 0x3;
		unsigned char delay = (blockId.state >> 2) + 1;
		return std::make_pair("powered_repeater", "delay=" + std::to_string(delay) + ",facing=" + dirs[dir] + ",locked=false");
	}
	case 99: {
		static const std::string variants[] = {
			"variant=all_inside",
			"variant=north_west",
			"variant=north",
			"variant=north_east",
			"variant=west",
			"variant=center",
			"variant=east",
			"variant=south_west",
			"variant=south",
			"variant=south_east",
			"variant=stem",
			"variant=all_outside",
			"variant=all_stem",
		};
		return std::make_pair("brown_mushroom_block", variants[blockId.state]);
	}
	case 100: {
		static const std::string variants[] = {
			"variant=all_inside",
			"variant=north_west",
			"variant=north",
			"variant=north_east",
			"variant=west",
			"variant=center",
			"variant=east",
			"variant=south_west",
			"variant=south",
			"variant=south_east",
			"variant=stem",
			"variant=all_outside",
			"variant=all_stem",
		};
		return std::make_pair("red_mushroom_block", variants[blockId.state]);
	}
	case 103: {
		return std::make_pair("melon_block", "normal");
	}
	case 106: {
		static const std::string values[] = {
			"false",
			"true",
		};
		return std::make_pair("vine", "east=" + values[(blockId.state >> 3) & 0x1] + ",north=" + values[(blockId.state >> 2) & 0x1] + ",south=" + values[blockId.state & 0x1] + ",up=" + values[blockId.state == 0] + ",west=" + values[(blockId.state >> 1) & 0x1]);
	}
	case 111: {
		return std::make_pair("waterlily", "normal");
	}
	case 112: {
		return std::make_pair("nether_brick", "normal");
	}
	case 121: {
		return std::make_pair("end_stone", "normal");
	}
	case 129: {
		return std::make_pair("emerald_ore", "normal");
	}
	case 133: {
		return std::make_pair("emerald_block", "normal");
	}
	case 141: {
		return std::make_pair("carrots", "age=" + std::to_string(blockId.state));
	}
	case 142: {
		return std::make_pair("potatoes", "age=" + std::to_string(blockId.state));
	}
	case 149: {
		static const std::string dirs[] = {
			"east",
			"south",
			"west",
			"north",
		};
		unsigned char dir = blockId.state & 0x3;
		bool substractMode = (blockId.state >> 2) & 0x1;
		bool isPowered = blockId.state >> 3;
		return std::make_pair("unpowered_comparator", "facing=" + dirs[dir] + ",mode=" + (substractMode ? "subtract" : "compare") + ",powered=" + (isPowered ? "true" : "false"));
	}
	case 153: {
		return std::make_pair("quartz_ore", "normal");
	}
	case 155: {
		return std::make_pair("quartz_block", "normal");
	}
	case 161: {
		if ((blockId.state & 0x3) > 2)
			break;
		static const std::pair<std::string, std::string> ids[] = {
			std::pair<std::string,std::string>("acacia_leaves", "normal"),
			std::pair<std::string,std::string>("dark_oak_leaves", "normal"),
		};
		return ids[blockId.state & 0x3];
	}
	case 162: {
		unsigned char type = blockId.state & 0x3;
		if (type > 2)
			break;
		unsigned char dir = (blockId.state & 0xC) >> 2;
		static const std::string types[] = {
			"acacia_log",
			"dark_oak_log",
		};
		static const std::string dirs[] = {
			"axis=y",
			"axis=x",
			"axis=z",
			"axis=none",
		};
		return std::make_pair(types[type], dirs[dir]);
	}
	case 175: {
		bool high = ((blockId.state >> 3) & 0x1);
		unsigned char type = blockId.state & 0x7;

		static const std::string types[] = {
			"sunflower",
			"syringa",
			"double_grass",
			"double_fern",
			"double_rose",
			"paeonia",
		};

		static const std::string isHigh[] = {
			"half=lower",
			"half=upper",
		};
		return std::make_pair(types[type], isHigh[high]);
	}
	case 207: {
		return std::make_pair("beetroots", "age=" + std::to_string(blockId.state));
	}
	case 208: {
		return std::make_pair("grass_path", "normal");
	}
	default:
		break;
	}
	
	return std::make_pair("", "");
}

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
