#pragma once

#include <vector>

#include <gl/glew.h>

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
	GLuint texture;
	std::vector<TextureCoord> textureCoords;
public:
	TextureAtlas(std::vector<TextureData> &textures);

	TextureAtlas(const TextureAtlas &) = delete;

	~TextureAtlas();

	inline GLuint GetRawTextureId() {
		return texture;
	}

	TextureCoord GetTexture(int id) {
		return textureCoords[id];
	}
};