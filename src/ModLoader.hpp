#pragma once
#include "AssetManager.hpp"

#include <experimental/filesystem>

namespace ModLoader {
	namespace fs = std::experimental::filesystem::v1;

	void LoadMod(AssetTreeNode &node);
	void LoadModels(AssetTreeNode &node);
	void LoadCode(AssetTreeNode &node);

	void ParseAssetTexture(AssetTreeNode &node);
	void ParseAssetBlockModel(AssetTreeNode &node);
	void ParseAssetBlockState(AssetTreeNode &node);
	void ParseAssetShader(AssetTreeNode &node);
	void ParseAssetLua(AssetTreeNode &node);

	void WalkDirEntry(const fs::directory_entry &dirEntry, AssetTreeNode *node);

	void RecursiveWalkAssetFiles(AssetTreeNode &assetNode, std::function<void(AssetTreeNode&)> fnc);
	void RecursiveWalkAssetPath(const std::string &assetPath, std::function<void(AssetTreeNode&)> fnc);
}
