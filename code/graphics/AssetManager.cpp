#include "AssetManager.hpp"

const std::string pathToAssets = "./assets/";
const std::string pathToObjects = pathToAssets + "objects/";
const std::string pathToIndexFile = pathToAssets + "indexes/1.11.json";
const std::string pathToAssetsMc = "./assetsMc/";

const std::map<Asset::AssetType, std::string> assetTypeFileExtensions{
        std::make_pair(Asset::AssetType::Texture, ".png"),
        std::make_pair(Asset::AssetType::Lang, ".lang"),
        std::make_pair(Asset::AssetType::Sound, ".ogg"),
};

AssetManager::AssetManager() {
    std::ifstream indexFile(pathToIndexFile);
    if (!indexFile) {
        std::cerr << "Can't open file " << pathToIndexFile << std::endl;
    }
    nlohmann::json json = nlohmann::json::parse(indexFile)["objects"];
    for (auto it = json.begin(); it != json.end(); ++it) {
        size_t fileNameExtensionPos = -1;
        std::string name = it.key();
        Asset::AssetType type = Asset::Unknown;
        for (auto &it:assetTypeFileExtensions) {
            if ((fileNameExtensionPos = name.find(it.second)) != std::string::npos) {
                type = it.first;
                name = name.substr(0, fileNameExtensionPos);
                break;
            }
        }
        std::string hash = it.value()["hash"].get<std::string>();
        size_t size = it.value()["size"].get<int>();
        Asset asset{name, hash, Asset::AssetData(), size, type};
        this->assets[name] = asset;
    }
}

AssetManager::~AssetManager() {

}

Asset &AssetManager::GetAsset(std::string AssetName) {
    if (instance().assets.find(AssetName) == instance().assets.end() || !instance().assets[AssetName].isParsed())
        LoadAsset(AssetName);
    return instance().assets[AssetName];
}

void AssetManager::LoadAsset(std::string AssetName) {
    if (instance().assets.find(AssetName) != instance().assets.end() && instance().assets[AssetName].isParsed())
        return;
    std::string AssetFileName = GetPathToAsset(AssetName);
    Asset &asset = instance().assets[AssetName];


    if (asset.type == Asset::Texture) {
        asset.data.texture = new Texture(AssetFileName,GL_CLAMP_TO_BORDER,GL_NEAREST);
        //asset.data.texture.loadFromFile((asset.name + assetTypeFileExtensions.at(asset.type)));
    }
}

std::string AssetManager::GetPathToAsset(std::string AssetName) {
    if (instance().assets.find(AssetName) != instance().assets.end()){
        auto it = instance().assets.find(AssetName);
        return pathToObjects + std::string(instance().assets[AssetName].hash.c_str(), 2) + "/" +
               instance().assets[AssetName].hash;
    }

    instance().assets[AssetName].hash="";
    instance().assets[AssetName].type=Asset::AssetType::Texture;
    instance().assets[AssetName].name=AssetName;
    instance().assets[AssetName].size=0;
    return pathToAssetsMc + "" + instance().assets[AssetName].name +
           assetTypeFileExtensions.at(instance().assets[AssetName].type);
}

std::string AssetManager::GetAssetNameByBlockId(unsigned short id) {
    std::string assetBase = "minecraft/textures/blocks/";
    std::string textureName;
    switch (id){
        case 0:
            textureName="air";
            break;
        case 1:
            textureName="stone";
            break;
        case 2:
            textureName="grass";
            break;
        case 3:
            textureName="dirt";
            break;
        case 16:
            textureName="coal_ore";
            break;
        case 17:
            textureName="log_oak";
            break;
        case 31:
            textureName="air";
            break;
        default:
            //std::cout<<id<<std::endl;
            textureName="beacon";
            break;
    }
    return assetBase+textureName;
}

bool Asset::isParsed() {
    switch (type) {
        case Unknown:
            return false;
            break;
        case Texture:
            return this->data.texture != nullptr;
            break;
        case Sound:
            return false;
            break;
        case Model:
            return false;
            break;
        case Lang:
            return false;
            break;
    }
}

Asset::~Asset() {
    switch (type) {
        case Unknown:
            break;
        case Texture:
            delete this->data.texture;
            break;
        case Sound:
            break;
        case Model:
            break;
        case Lang:
            break;
    }
}
