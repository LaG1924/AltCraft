#pragma once

#include <experimental/filesystem>
#include <map>
#include <GL/glew.h>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>
#include "../world/Block.hpp"
#include "../graphics/Texture.hpp"

struct TextureCoord{
    unsigned int x,y,w,h;
};

class AssetManager {
    Texture *textureAtlas;
    std::map<std::string,Block> assetIds;
    std::map<std::string,TextureCoord> assetTextures;
public:
    AssetManager();

    ~AssetManager();

    void LoadTextureResources();

    TextureCoord GetTextureByAssetName(std::string AssetName);

    std::string GetTextureAssetNameByBlockId(unsigned short BlockId, unsigned char BlockSide = 0);

    const GLuint GetTextureAtlas();

    void LoadIds();
};
