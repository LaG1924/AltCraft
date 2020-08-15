#pragma once
#include "AssetManager.hpp"

#include <experimental/filesystem>

namespace ModLoader {
	namespace fs = std::experimental::filesystem::v1;
	struct Mod{
		std::string modid;
		std::string dirname;
		enum{
			resourcepack,
			lua
		}type;
		std::string name, version;
		std::string description;
		std::vector<std::string> authors;
		std::string url;
	};

	void LoadMod(AssetTreeNode &node) noexcept;
	void LoadModels(AssetTreeNode &node) noexcept;
	void LoadCode(AssetTreeNode &node) noexcept;
	void LoadModinfo(AssetTreeNode &node) noexcept;
	void LoadMcmeta(AssetTreeNode &node);

	void ParseAssetTexture(AssetTreeNode &node) noexcept;
	void ParseAssetBlockModel(AssetTreeNode &node) noexcept;
	void ParseAssetBlockState(AssetTreeNode &node) noexcept;
	void ParseAssetShader(AssetTreeNode &node) noexcept;
	void ParseAssetLua(AssetTreeNode &node) noexcept;
	void ParseAssetSound(AssetTreeNode &node) noexcept;

	void WalkDirEntry(const fs::directory_entry &dirEntry, AssetTreeNode *node) noexcept;

	void RecursiveWalkAssetFiles(AssetTreeNode &assetNode, std::function<void(AssetTreeNode&)> fnc) noexcept;
	void RecursiveWalkAssetPath(const std::string &assetPath, std::function<void(AssetTreeNode&)> fnc) noexcept;

	std::shared_ptr<Mod> GetModByModid(const std::string &modid) noexcept;
	std::shared_ptr<Mod> GetModByDirName(const std::string &name) noexcept;
}
