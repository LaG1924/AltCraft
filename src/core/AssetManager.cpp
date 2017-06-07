
#include "AssetManager.hpp"

namespace fs = std::experimental::filesystem;

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
            {BlockTextureId(0, 0),    "minecraft/textures/blocks/air"},
            {BlockTextureId(1, 0),    "minecraft/textures/blocks/stone"},
            {BlockTextureId(1, 1),    "minecraft/textures/blocks/stone_granite"},

            {BlockTextureId(2, 0, 0), "minecraft/textures/blocks/dirt"},
            {BlockTextureId(2, 0, 1), "minecraft/textures/blocks/grass_top"},
            {BlockTextureId(2, 0, 2), "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 3), "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 4), "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 5), "minecraft/textures/blocks/grass_side"},

            {BlockTextureId(3, 0),    "minecraft/textures/blocks/dirt"},
            {BlockTextureId(4, 0),    "minecraft/textures/blocks/cobblestone"},
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
