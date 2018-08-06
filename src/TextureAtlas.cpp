#include "TextureAtlas.hpp"

#define STB_RECT_PACK_IMPLEMENTATION

#include <stb_rect_pack.h>
#include <easylogging++.h>

#include "Utility.hpp"

TextureAtlas::TextureAtlas(std::vector<TextureData> &textures) {
	LOG(INFO) << "Initializing texture atlas...";
	LOG(INFO) << "Textures count: " << textures.size();

	//Texture packing
	const int textureSize = 1024;
	const int padding = 1;
	const int paddingLimit = 128;

	std::vector<stbrp_rect> totalRects;
	for (int i = 0; i < textures.size(); i++) {
		stbrp_rect rect;
		rect.id = i;
		rect.x = 0;
		rect.y = 0;
		if (textures[i].width >= paddingLimit || textures[i].height >= paddingLimit) {
			rect.w = textures[i].width;
			rect.h = textures[i].height;
		} else {
			rect.w = textures[i].width + padding * 2;
			rect.h = textures[i].height + padding * 2;
		}
		rect.was_packed = 0;
		totalRects.push_back(rect);
	}

	textureCoords.resize(textures.size());

	int layer = 0;
	for (;;layer++) {
		stbrp_context context;
		std::vector<stbrp_node> nodes;
		int nodesCount = textureSize * 2;
		nodes.resize(nodesCount);
		stbrp_init_target(&context, textureSize, textureSize, nodes.data(), nodesCount);

		std::vector<stbrp_rect> rects;
		for (const auto &it : totalRects) {
			if (it.was_packed != 0)
				continue;

			rects.push_back(it);
		}
		stbrp_pack_rects(&context, rects.data(), rects.size());

		int unpackedTextures = 0;
		for (auto &it : rects) {
			if (!it.was_packed) {
				unpackedTextures++;
				continue;
			}
			if (it.w >= paddingLimit || it.h >= paddingLimit) {
				textureCoords[it.id].pixelX = it.x;
				textureCoords[it.id].pixelY = it.y;
				textureCoords[it.id].pixelW = it.w;
				textureCoords[it.id].pixelH = it.h;
			} else {
				textureCoords[it.id].pixelX = it.x + padding;
				textureCoords[it.id].pixelY = it.y + padding;
				textureCoords[it.id].pixelW = it.w - padding * 2;
				textureCoords[it.id].pixelH = it.h - padding * 2;
			}
			textureCoords[it.id].x = (double)textureCoords[it.id].pixelX / textureSize;
			textureCoords[it.id].y = (double)textureCoords[it.id].pixelY / textureSize;
			textureCoords[it.id].w = (double)textureCoords[it.id].pixelW / textureSize;
			textureCoords[it.id].h = (double)textureCoords[it.id].pixelH / textureSize;
			textureCoords[it.id].layer = layer;
			totalRects[it.id].was_packed = 1;
		}
		if (unpackedTextures == 0)
			break;
	}
	LOG(INFO) << "Texture atlas size is " << textureSize << "x" << textureSize << "x" << layer;

	//OpenGL
	int mipLevelCount = 1;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, textureSize, textureSize, layer+1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCheckError();

	//Uploading texture data
	for (int i = 0; i < textureCoords.size(); i++) {
		size_t bytesPerLine = textureCoords[i].pixelW * 4;
		for (int y = 0; y < textureCoords[i].pixelH / 2; y++) {
			int invY = textureCoords[i].pixelH - y - 1;
			unsigned char *src = textures[i].data.data() + y * bytesPerLine;
			unsigned char *dst = textures[i].data.data() + invY * bytesPerLine;
			for (int j = 0; j < bytesPerLine; j++) {
				std::swap(*(src + j), *(dst + j));
			}
		}
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, textureCoords[i].pixelX, textureSize - textureCoords[i].pixelY - textureCoords[i].pixelH, textureCoords[i].layer,
			textureCoords[i].pixelW, textureCoords[i].pixelH, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, textures[i].data.data());
		glCheckError();
	}

	LOG(INFO) << "Texture atlas initialized";
}

TextureAtlas::~TextureAtlas() {
	glDeleteTextures(1, &texture);
}
