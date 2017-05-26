#pragma once

#include <fstream>
#include <string>
#include <map>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include "Texture.hpp"

struct Asset {
    std::string name = "";
    std::string hash = "";
    union AssetData{
        Texture *texture;
    } data;
    size_t size = 0;
    enum AssetType {
        Unknown,
        Texture,
        Sound,
        Model,
        Lang,
    } type = Unknown;
    bool isParsed();
    ~Asset();
};

class AssetManager_old {
    AssetManager_old();

    ~AssetManager_old();

    AssetManager_old(const AssetManager_old &);

    AssetManager_old &operator=(const AssetManager_old &);

    std::map<std::string, Asset> assets;

    static AssetManager_old &instance() {
        static AssetManager_old assetManager;
        return assetManager;
    }

    static std::string GetPathToAsset(std::string AssetName);
public:

    static Asset &GetAsset(std::string AssetName);

    static void LoadAsset(std::string AssetName);

    static std::string GetAssetNameByBlockId(unsigned short id);
};

