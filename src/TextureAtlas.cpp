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

	size_t layer = 0;
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

	//Gal
	int mipLevelCount = 1;

	auto gal = Gal::GetImplementation();
	auto texConfig = gal->CreateTexture3DConfig(textureSize, textureSize, layer + 1, false, Gal::Format::R8G8B8A8);
	texConfig->SetWrapping(Gal::Wrapping::Clamp);
	texConfig->SetMinFilter(Gal::Filtering::Nearest);
	texConfig->SetMaxFilter(Gal::Filtering::Nearest);
	texConfig->SetLinear(false);

	texture = gal->BuildTexture(texConfig);

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
		texture->SetSubData(
			textureCoords[i].pixelX,
			textureSize - textureCoords[i].pixelY - textureCoords[i].pixelH,
			textureCoords[i].layer,
			textureCoords[i].pixelW,
			textureCoords[i].pixelH,
			1,
			{ reinterpret_cast<std::byte*>(textures[i].data.data()), reinterpret_cast<std::byte*>(textures[i].data.data()) + textures[i].data.size() }
		);
	}

	LOG(INFO) << "Texture atlas initialized";
}
