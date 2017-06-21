#pragma once

#include <experimental/filesystem>
#include <map>

#include <GL/glew.h>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>

#include <world/Block.hpp>
#include <graphics/Texture.hpp>

struct TextureCoordinates {
	TextureCoordinates(float x = -1, float y = -1, float w = -1, float h = -1) : x(x), y(y), w(w), h(h) {}

	bool operator==(const TextureCoordinates &rhs) const {
		return x == rhs.x &&
		       y == rhs.y &&
		       w == rhs.w &&
		       h == rhs.h;
	}

	explicit operator bool() const {
		return !(*this == TextureCoordinates(-1, -1, -1, -1));
	}

	float x, y, w, h;
};

struct BlockTextureId {
	//Block sides: 0 - bottom, 1 - top, 2 - north, 3 - south, 4 - west, 5 - east 6 - every side
	BlockTextureId(int id = 0, int state = 0, int side = 6) : id(id), state(state), side(side) {}

	int id:9;
	int state:4;
	int side:3;


	bool operator<(const BlockTextureId &rhs) const {
		if (id < rhs.id)
			return true;
		if (rhs.id < id)
			return false;
		if (state < rhs.state)
			return true;
		if (rhs.state < state)
			return false;
		return side < rhs.side;
	}
};

class AssetManager {
	Texture *textureAtlas;
	std::map<std::string, Block> assetIds;
	std::map<std::string, TextureCoordinates> assetTextures;
public:
	AssetManager();

	~AssetManager();

	void LoadTextureResources();

	TextureCoordinates GetTextureByAssetName(std::string AssetName);

	std::string GetTextureAssetNameByBlockId(BlockTextureId block);

	GLuint GetTextureAtlas();

	void LoadIds();

	TextureCoordinates GetTextureByBlock(BlockTextureId block);
};
