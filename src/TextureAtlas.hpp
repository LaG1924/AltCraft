#pragma once

#include <vector>

#include "Gal.hpp"

struct TextureData {
	std::vector<unsigned char> data; //expected format RGBA8888
	int width, height;
};

struct TextureCoord {
	double x, y, w, h;
	int pixelX, pixelY, pixelW, pixelH;
	size_t layer;
};

class TextureAtlas {
	std::shared_ptr<Gal::Texture> texture;
	std::vector<TextureCoord> textureCoords;
public:
	TextureAtlas(std::vector<TextureData> &textures);

	std::shared_ptr<Gal::Texture> GetGalTexture() {
		return texture;
	}

	TextureCoord GetTexture(int id) {
		return textureCoords[id];
	}
};