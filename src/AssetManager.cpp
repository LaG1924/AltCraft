#include <fstream>
#include "AssetManager.hpp"

//const fs::path pathToAssets = "./assets/";
//const fs::path pathToAssetsList = "./items.json";
//const fs::path pathToTextureIndex = "./textures.json";
const std::string pathToAssetsList = "./items.json";
const std::string pathToTextureIndex = "./textures.json";

AssetManager::AssetManager() {
	LoadIds();
	LoadTextureResources();
}

void AssetManager::LoadIds() {
	std::ifstream in(pathToAssetsList);
	nlohmann::json index;
	in >> index;
	for (auto &it:index) {
		int id = it["type"].get<int>();
		int state = it["meta"].get<int>();
		std::string blockName = it["text_type"].get<std::string>();
		assetIds[blockName] = Block(id, state);
	}
	LOG(INFO) << "Loaded " << assetIds.size() << " ids";
}

AssetManager::~AssetManager() {
	delete textureAtlas;
}

//TODO: This function must be replaced with runtime texture atlas generating
void AssetManager::LoadTextureResources() {
	std::ifstream in(pathToTextureIndex);
	nlohmann::json index;
	in >> index;
	std::string filename = index["meta"]["image"].get<std::string>();
	float textureWidth = index["meta"]["size"]["w"].get<int>();
	float textureHeight = index["meta"]["size"]["h"].get<int>();
	for (auto &it:index["frames"]) {
		auto frame = it["frame"];
		TextureCoordinates coord;
		coord.x = frame["x"].get<int>() / textureWidth;
		coord.y = frame["y"].get<int>() / textureHeight;
		coord.w = frame["w"].get<int>() / textureWidth;
		coord.h = frame["h"].get<int>() / textureHeight;
		std::string assetName = it["filename"].get<std::string>();
		assetName.insert(0, "minecraft/textures/");
		assetName.erase(assetName.length() - 4);
		assetTextures[assetName] = coord;
	}

	textureAtlas = new Texture(filename);
	LOG(INFO) << "Texture atlas id is " << textureAtlas->texture;
}

TextureCoordinates AssetManager::GetTextureByAssetName(std::string AssetName) {
	if (assetTextures.find(AssetName) != assetTextures.end())
		return assetTextures[AssetName];
	else
		return TextureCoordinates{-1, -1, -1, -1};
}

std::string AssetManager::GetTextureAssetNameByBlockId(BlockTextureId block) {
	//Block sides: 0 - bottom, 1 - top, 2 - north, 3 - south, 4 - west, 5 - east 6 - every side
	const std::map<BlockTextureId, std::string> lookupTable = {
			{BlockTextureId(0, 0),     "minecraft/textures/blocks/air"},
			{BlockTextureId(1, 0),     "minecraft/textures/blocks/stone"},
			{BlockTextureId(1, 1),     "minecraft/textures/blocks/stone_granite"},

			{BlockTextureId(2, 0, 0),  "minecraft/textures/blocks/dirt"},
			{BlockTextureId(2, 0, 1),  "minecraft/textures/blocks/grass_top"},
			{BlockTextureId(2, 0, 2),  "minecraft/textures/blocks/grass_side"},
			{BlockTextureId(2, 0, 3),  "minecraft/textures/blocks/grass_side"},
			{BlockTextureId(2, 0, 4),  "minecraft/textures/blocks/grass_side"},
			{BlockTextureId(2, 0, 5),  "minecraft/textures/blocks/grass_side"},

			{BlockTextureId(3, 0),     "minecraft/textures/blocks/dirt"},
			{BlockTextureId(4, 0),     "minecraft/textures/blocks/cobblestone"},
			{BlockTextureId(5, 0),     "minecraft/textures/blocks/planks"},

			{BlockTextureId(7, 0),     "minecraft/textures/blocks/bedrock"},

			{BlockTextureId(17, 0, 0), "minecraft/textures/blocks/log_oak_top"},
			{BlockTextureId(17, 0, 1), "minecraft/textures/blocks/log_oak_top"},
			{BlockTextureId(17, 0, 2), "minecraft/textures/blocks/log_oak"},
			{BlockTextureId(17, 0, 3), "minecraft/textures/blocks/log_oak"},
			{BlockTextureId(17, 0, 4), "minecraft/textures/blocks/log_oak"},
			{BlockTextureId(17, 0, 5), "minecraft/textures/blocks/log_oak"},

			{BlockTextureId(17, 1, 0), "minecraft/textures/blocks/log_spruce_top"},
			{BlockTextureId(17, 1, 1), "minecraft/textures/blocks/log_spruce_top"},
			{BlockTextureId(17, 1, 2), "minecraft/textures/blocks/log_spruce"},
			{BlockTextureId(17, 1, 3), "minecraft/textures/blocks/log_spruce"},
			{BlockTextureId(17, 1, 4), "minecraft/textures/blocks/log_spruce"},
			{BlockTextureId(17, 1, 5), "minecraft/textures/blocks/log_spruce"},

			{BlockTextureId(17, 2, 0), "minecraft/textures/blocks/log_birch_top"},
			{BlockTextureId(17, 2, 1), "minecraft/textures/blocks/log_birch_top"},
			{BlockTextureId(17, 2, 2), "minecraft/textures/blocks/log_birch"},
			{BlockTextureId(17, 2, 3), "minecraft/textures/blocks/log_birch"},
			{BlockTextureId(17, 2, 4), "minecraft/textures/blocks/log_birch"},
			{BlockTextureId(17, 2, 5), "minecraft/textures/blocks/log_birch"},

			{BlockTextureId(17, 3, 0), "minecraft/textures/blocks/log_jungle_top"},
			{BlockTextureId(17, 3, 1), "minecraft/textures/blocks/log_jungle_top"},
			{BlockTextureId(17, 3, 2), "minecraft/textures/blocks/log_jungle"},
			{BlockTextureId(17, 3, 3), "minecraft/textures/blocks/log_jungle"},
			{BlockTextureId(17, 3, 4), "minecraft/textures/blocks/log_jungle"},
			{BlockTextureId(17, 3, 5), "minecraft/textures/blocks/log_jungle"},

			{BlockTextureId(18, 0),    "minecraft/textures/blocks/leaves_oak"},
			{BlockTextureId(18, 1),    "minecraft/textures/blocks/leaves_spruce"},
			{BlockTextureId(18, 2),    "minecraft/textures/blocks/leaves_birch"},
			{BlockTextureId(18, 3),    "minecraft/textures/blocks/leaves_jungle"},

			{BlockTextureId(61, 0, 0), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(61, 0, 1), "minecraft/textures/blocks/furnace_top"},
			{BlockTextureId(61, 0, 2), "minecraft/textures/blocks/furnace_front_off"},
			{BlockTextureId(61, 0, 3), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(61, 0, 4), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(61, 0, 5), "minecraft/textures/blocks/furnace_side"},

			{BlockTextureId(62, 0, 0), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(62, 0, 1), "minecraft/textures/blocks/furnace_top"},
			{BlockTextureId(62, 0, 2), "minecraft/textures/blocks/furnace_front_on"},
			{BlockTextureId(62, 0, 3), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(62, 0, 4), "minecraft/textures/blocks/furnace_side"},
			{BlockTextureId(62, 0, 5), "minecraft/textures/blocks/furnace_side"},


			{BlockTextureId(31, 0),    "minecraft/textures/blocks/deadbush"},
			{BlockTextureId(31, 1),    "minecraft/textures/blocks/tallgrass"},
			{BlockTextureId(31, 2),    "minecraft/textures/blocks/fern"},
	};
	auto ret = lookupTable.find(block);
	if (ret == lookupTable.end())
		return "";
	else
		return ret->second;
}

GLuint AssetManager::GetTextureAtlas() {
	return textureAtlas->texture;
}

TextureCoordinates AssetManager::GetTextureByBlock(BlockTextureId block) {
	std::string assetName = this->GetTextureAssetNameByBlockId(block);
	return this->GetTextureByAssetName(assetName);
}

const std::map<BlockTextureId, glm::vec4> &AssetManager::GetTextureAtlasIndexes() {
	if (!textureAtlasIndexes.empty())
		return textureAtlasIndexes;

	LOG(INFO) << "Initializing texture atlas...";
	for (int id = 1; id < 128; id++) {
		for (int state = 0; state < 16; state++) {
			BlockTextureId blockTextureId(id, state, 6);
			if (!this->GetTextureByBlock(blockTextureId) &&
			    !this->GetTextureByBlock(BlockTextureId(id, state, 0))) {
				continue;
			}
			if (this->GetTextureByBlock(blockTextureId)) {
				for (int i = 0; i < 6; i++) {
					TextureCoordinates tc = this->GetTextureByBlock(BlockTextureId(id, state, 6));
					textureAtlasIndexes[BlockTextureId(id, state, i)] = glm::vec4(tc.x, tc.y, tc.w, tc.h);
				}
			} else {
				for (int i = 0; i < 6; i++) {
					TextureCoordinates tc = this->GetTextureByBlock(BlockTextureId(id, state, i));
					textureAtlasIndexes[BlockTextureId(id, state, i)] = glm::vec4(tc.x, tc.y, tc.w, tc.h);
				}
			}
		}
	}
	LOG(INFO) << "Created " << textureAtlasIndexes.size() << " texture indexes";

	return textureAtlasIndexes;
}

AssetManager &AssetManager::Instance() {
	static AssetManager assetManager;
	return assetManager;
}
