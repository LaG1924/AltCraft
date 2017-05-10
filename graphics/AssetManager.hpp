#pragma once

#include <fstream>
#include <string>
#include <SOIL.h>
#include <map>
#include "../json.hpp"

struct Asset {
    std::string name = "";
    std::string hash = "";
    union AssetData{
        struct TextureData{
            int width;
            int height;
            unsigned char *imageData;
        } texture;
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

class AssetManager {
    AssetManager();

    ~AssetManager();

    AssetManager(const AssetManager &);

    AssetManager &operator=(const AssetManager &);

    std::map<std::string, Asset> assets;
public:
    static AssetManager &instance() {
        static AssetManager assetManager;
        return assetManager;
    }

    static Asset &GetAsset(std::string AssetName);

    static void LoadAsset(std::string AssetName);
};

