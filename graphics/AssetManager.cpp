#include "AssetManager.hpp"

const std::string pathToIndexFile = "./assets/indexes/1.11.json";

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
    Asset &asset = instance().assets[AssetName];
    if (!asset.isParsed())
        LoadAsset(AssetName);
    return asset;
}

void AssetManager::LoadAsset(std::string AssetName) {
    if (instance().assets[AssetName].isParsed())
        return;
    Asset &asset = instance().assets[AssetName];
    if (asset.type == Asset::Texture) {
        asset.data.texture.imageData = SOIL_load_image((asset.name + assetTypeFileExtensions.at(asset.type)).c_str(),
                                                       &asset.data.texture.width, &asset.data.texture.height, 0,
                                                       SOIL_LOAD_RGBA);
    }
}

bool Asset::isParsed() {
    switch (type) {
        case Unknown:
            return false;
            break;
        case Texture:
            return this->data.texture.imageData != nullptr;
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
            SOIL_free_image_data(this->data.texture.imageData);
            break;
        case Sound:
            break;
        case Model:
            break;
        case Lang:
            break;
    }
}
