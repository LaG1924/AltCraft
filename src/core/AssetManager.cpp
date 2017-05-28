
#include "AssetManager.hpp"

namespace fs = std::experimental::filesystem;

const fs::path pathToAssets = "./assets/";
const fs::path pathToAssetsList = "./items.json";
const fs::path pathToTextureIndex = "./textures.json";

AssetManager::AssetManager() {
    for (auto &it:fs::recursive_directory_iterator(pathToAssets)) {

    }
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
        assetIds[blockName] = Block(id, 0, state);
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
    for (auto &it:index["frames"]) {
        auto frame = it["frame"];
        TextureCoord coord;
        coord.x = frame["x"].get<int>();
        coord.y = frame["y"].get<int>();;
        coord.w = frame["w"].get<int>();
        coord.h = frame["h"].get<int>();
        std::string assetName = it["filename"].get<std::string>();
        assetName.insert(0, "minecraft/textures/");
        assetName.erase(assetName.length() - 4);
        assetTextures[assetName] = coord;
    }

    textureAtlas = new Texture(filename);
    LOG(INFO) << "Texture atlas id is " << textureAtlas->texture;
}

TextureCoord AssetManager::GetTextureByAssetName(std::string AssetName) {
    return assetTextures[AssetName];
}

std::string AssetManager::GetTextureAssetNameByBlockId(unsigned short BlockId, unsigned char BlockSide) {
    //Block sides: 0 - bottom, 1 - top, 2 - north, 3 - south, 4 - west, 5 - east
    std::map<Block, std::string> lookupTable = {
            {Block(0),       "minecraft/textures/blocks/air"},
            {Block(1, 0),    "minecraft/textures/blocks/stone"},
            {Block(1, 1),    "minecraft/textures/blocks/stone_granite"},

            {Block(2, 0, 0), "minecraft/textures/blocks/dirt"},
            {Block(2, 0, 1), "minecraft/textures/blocks/grass_top"},
            {Block(2, 0, 2), "minecraft/textures/blocks/grass_side"},
            {Block(2, 0, 3), "minecraft/textures/blocks/grass_side"},
            {Block(2, 0, 4), "minecraft/textures/blocks/grass_side"},
            {Block(2, 0, 5), "minecraft/textures/blocks/grass_side"},

            {Block(3),       "minecraft/textures/blocks/dirt"},
            {Block(4),       "minecraft/textures/blocks/cobblestone"},
    };
    return lookupTable[Block(BlockId, BlockSide)];
}

const GLuint AssetManager::GetTextureAtlas() {
    return textureAtlas->texture;
}
