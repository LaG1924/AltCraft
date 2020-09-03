#include "AssetManager.hpp"
#include "ModLoader.hpp"

#include <fstream>
#include <experimental/filesystem>

#include <nlohmann/json.hpp>
#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>
#include <optick.h>

#include "Utility.hpp"
#include "Plugin.hpp"

namespace fs = std::experimental::filesystem::v1;

const fs::path pathToAssets = "./assets/";
const std::string pathToAssetsList = "./items.json";

std::map<std::string, BlockId> assetIds;
std::map<BlockId, std::string> blockIdToBlockName;
std::unique_ptr<AssetTreeNode> assetTree;
std::unique_ptr<TextureAtlas> atlas;
std::map<BlockId, BlockFaces> blockIdToBlockFaces;

BlockFaces errorFaces;

void LoadIds();
void LoadAssets();
void LoadTextures();
void LoadScripts();

void ParseBlockModels();

void AssetManager::InitAssetManager()
{
	static bool Initialized = false;
	if (Initialized) {
		LOG(WARNING) << "Trying to init AssetManager twice";
	}
	Initialized = true;
	
	LoadAssets();

	AssetTreeNode *assetNode = GetAssetByAssetName("/");
	for (auto& it : assetNode->childs) {
		ModLoader::LoadMod(*it.get());
	}

	LoadTextures();

	LoadIds();
	ParseBlockModels();

	PluginSystem::Init();
	LoadScripts();

	AssetTreeNode *errorBlock = GetAssetByAssetName("/minecraft/models/block/error");
	errorFaces.transform = glm::mat4(1.0);
	if (errorBlock && errorBlock->type == AssetTreeNode::ASSET_BLOCK_MODEL) {
		AssetBlockModel *blockModel = reinterpret_cast<AssetBlockModel*>(errorBlock->asset.get());
		errorFaces.faces = blockModel->blockModel.parsedFaces;
		errorFaces.isBlock = blockModel->blockModel.IsBlock;
	}
	for (int i = 0; i < FaceDirection::none; i++) {
		errorFaces.faceDirectionVector[i] = FaceDirectionVector[i];
	}
}





void LoadIds() {
	std::ifstream in(pathToAssetsList);
	nlohmann::json index;
	in >> index;
	for (auto &it : index) {
		unsigned short id = it["type"].get<int>();
		unsigned char state = it["meta"].get<int>();
		std::string blockName = it["text_type"].get<std::string>();
		assetIds[blockName] = BlockId{ id, state };
	}
	LOG(INFO) << "Loaded " << assetIds.size() << " ids";
}

void LoadAssets() {
	assetTree = std::make_unique<AssetTreeNode>();
	assetTree->name = "/";
	ModLoader::WalkDirEntry(fs::directory_entry(pathToAssets), assetTree.get());
}

void LoadTextures() {
	std::vector<TextureData> textureData;
	size_t id = 0;
	ModLoader::RecursiveWalkAssetPath("/minecraft/textures/", [&](AssetTreeNode &node) {
		TextureData data;
		if (node.type != AssetTreeNode::ASSET_TEXTURE)
			return;
		AssetTexture *textureAsset = reinterpret_cast<AssetTexture*>(node.asset.get());
		data.data = std::move(textureAsset->textureData);
		data.width = textureAsset->realWidth;
		data.height = textureAsset->realHeight;
		textureData.push_back(data);
		textureAsset->id = id++;
	});
	atlas = std::make_unique<TextureAtlas>(textureData);
}

void LoadScripts() {
	AssetTreeNode *node = AssetManager::GetAssetByAssetName("/");
	for (auto &it : node->childs) {
		for (auto &chld : it->childs) {
			if (chld->name == "code") {
				for (auto &child : chld->childs) {
					if (child->name == "lua") {
						for (auto &script : child->childs)
						{
							if (script->name != "init")
								continue;

//							Assume that /module/code/lua ASSET_SCRIPT
							AssetScript *asset = reinterpret_cast<AssetScript *>(script->asset.get());
							if (!asset) {
								LOG(ERROR) << "Unrecognised script file /" << it->name;
								continue;
							}
							try {
								PluginSystem::Execute(asset->code, true);
							}
							catch (std::exception & e) {
								LOG(ERROR) << "Failed loading script '" << script->name << "' in '" << it->name << "'";
							}

							break;
						}
					}
				}
			}
		}
	}
	LOG(INFO) << "Scripts loaded";
}


void ParseBlockModels() {
	std::string textureName;

	auto parseBlockModel = [&](AssetTreeNode &node) {
		if (!node.asset || node.type != AssetTreeNode::ASSET_BLOCK_MODEL)
			return;

		BlockModel &model = dynamic_cast<AssetBlockModel*>(node.asset.get())->blockModel;
		for (const auto& element : model.Elements) {
			Vector t = element.to - element.from;
			VectorF elementSize(VectorF(t.x, t.y, t.z) / 16.0f);
			VectorF elementOrigin(VectorF(element.from.x, element.from.y, element.from.z) / 16.0f);

			glm::mat4 elementTransform = glm::mat4(1.0);

			if (element.rotationAngle != 0) {
				static const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
				static const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
				static const glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

				const glm::vec3 *targetAxis = nullptr;
				switch (element.rotationAxis) {
				case BlockModel::ElementData::Axis::x:
					targetAxis = &xAxis;
					break;
				case BlockModel::ElementData::Axis::y:
					targetAxis = &yAxis;
					break;
				case BlockModel::ElementData::Axis::z:
					targetAxis = &zAxis;
					break;
				}

				VectorF rotateOrigin(VectorF(element.rotationOrigin.x, element.rotationOrigin.y, element.rotationOrigin.z) / 16.0f);

				glm::mat4 rotationMat = glm::mat4(1.0);
				rotationMat = glm::translate(rotationMat, rotateOrigin.glm());

				rotationMat = glm::rotate(rotationMat, glm::radians((float)element.rotationAngle), *targetAxis);
				if (element.rotationRescale) {
					glm::vec3 scaleFactor{ 1.0f,1.0f,1.0f };
					double coef = 1.0f / cos(glm::radians((double)element.rotationAngle));
					switch (element.rotationAxis) {
					case BlockModel::ElementData::Axis::x:
						scaleFactor.y *= coef;
						scaleFactor.z *= coef;
						break;
					case BlockModel::ElementData::Axis::y:
						scaleFactor.x *= coef;
						scaleFactor.z *= coef;
						break;
					case BlockModel::ElementData::Axis::z:
						scaleFactor.x *= coef;
						scaleFactor.y *= coef;
						break;
					}
					rotationMat = glm::scale(rotationMat, scaleFactor);
				}

				rotationMat = glm::translate(rotationMat, -rotateOrigin.glm());

				elementTransform = rotationMat * elementTransform;
			}

			elementTransform = glm::translate(elementTransform, elementOrigin.glm());
			elementTransform = glm::scale(elementTransform, elementSize.glm());

			for (const auto& face : element.faces) {
				ParsedFace parsedFace;
				parsedFace.visibility = face.second.cullface;

				glm::mat4 faceTransform = glm::mat4(1.0);
				switch (face.first) {
				case FaceDirection::down:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0, 0));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					faceTransform = glm::translate(faceTransform, glm::vec3(0.5f, 0.0f, 0.5f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(-0.5f, 0.0f, -0.5f));
					break;
				case FaceDirection::up:
					faceTransform = glm::translate(elementTransform, glm::vec3(0.0f, 1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0.5f, 0.0f, 0.5f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(-0.5f, 0.0f, -0.5f));
					break;
				case FaceDirection::north:
					faceTransform = glm::translate(elementTransform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					break;
				case FaceDirection::south:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 1));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1, 0, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1.0f));
					break;
				case FaceDirection::east:
					faceTransform = glm::translate(elementTransform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					break;
				case FaceDirection::west:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					break;
				default:
					break;
				}
				parsedFace.transform = faceTransform;
				TextureCoord texture;
				unsigned int textureFrames = 1;
				textureName = face.second.texture;
				if (model.Textures.empty()) {
					texture = AssetManager::GetTexture("minecraft/textures/error");
				}
				else {
					while (textureName[0] == '#') {
						textureName.erase(0, 1);
						auto textureIt = model.Textures.find(textureName);
						textureName = textureIt != model.Textures.end() ? textureIt->second : "minecraft/textures/error";
					}
					textureName.insert(0, "/minecraft/textures/");
					AssetTreeNode *node = AssetManager::GetAssetByAssetName(textureName);
					AssetTexture *assetTexture = nullptr;
					if (!node || node->type == AssetTreeNode::ASSET_TEXTURE)
						assetTexture = reinterpret_cast<AssetTexture*>(node->asset.get());

					texture = atlas->GetTexture(assetTexture->id);
					textureFrames = assetTexture->frames;

					if (!(face.second.uv == BlockModel::ElementData::FaceData::Uv{ 0,16,0,16 }) && !(face.second.uv == BlockModel::ElementData::FaceData::Uv{ 0,0,0,0 })
						&& !(face.second.uv == BlockModel::ElementData::FaceData::Uv{ 0,0,16,16 })) {
						double x = face.second.uv.x1;
						double y = face.second.uv.x1;
						double w = face.second.uv.x2 - face.second.uv.x1;
						double h = face.second.uv.y2 - face.second.uv.y1;
						x /= 16.0;
						y /= 16.0;
						w /= 16.0;
						h /= 16.0;
						double X = texture.x;
						double Y = texture.y;
						double W = texture.w;
						double H = texture.h;

						texture.x = X + x * W;
						texture.y = Y + y * H;
						texture.w = w * W;
						texture.h = h * H;
					}
				}
				parsedFace.texture = glm::vec4{ texture.x,texture.y,texture.w,texture.h };
				parsedFace.layer = texture.layer;
				parsedFace.frames = textureFrames;
				if (face.second.tintIndex)
					parsedFace.color = glm::vec3(0.275, 0.63, 0.1);
				else
					parsedFace.color = glm::vec3(0, 0, 0);

				model.parsedFaces.push_back(parsedFace);
			}
		}
	};

	ModLoader::RecursiveWalkAssetPath("/minecraft/models/", parseBlockModel);
}

BlockFaces &AssetManager::GetBlockModelByBlockId(BlockId block) {
	auto it = blockIdToBlockFaces.find(block);
	if (it != blockIdToBlockFaces.end())
		return it->second;

	BlockInfo *blockInfo = GetBlockInfo(block);
	AssetTreeNode *node = GetAssetByAssetName("/minecraft/blockstates/" + blockInfo->blockstate);
	AssetBlockState *asset = reinterpret_cast<AssetBlockState*>(node->asset.get());
	if (!asset || node->type != AssetTreeNode::ASSET_BLOCK_STATE)
		return errorFaces;
	
	BlockState &blockState = asset->blockState;
	if (blockState.variants.find(blockInfo->variant) == blockState.variants.end())
		return errorFaces;

	BlockStateVariant &variant = blockState.variants[blockInfo->variant];
	if (variant.models.empty())
		return errorFaces;

	BlockStateVariant::Model &model = variant.models[0];
	node = GetAssetByAssetName("/minecraft/models/block/" + model.modelName);
	AssetBlockModel *assetModel = reinterpret_cast<AssetBlockModel*>(node->asset.get());
	if (!assetModel || node->type != AssetTreeNode::ASSET_BLOCK_MODEL)
		return errorFaces;
	
	BlockFaces blockFaces;
	blockFaces.transform = glm::mat4(1.0);
	blockFaces.faces = assetModel->blockModel.parsedFaces;
	blockFaces.isBlock = assetModel->blockModel.IsBlock;
	glm::mat4 transform = glm::mat4(1.0);

	if (model.y != 0) {
		blockFaces.transform = glm::translate(blockFaces.transform, glm::vec3(0.5f, 0.0f, 0.5f));
		blockFaces.transform = glm::rotate(blockFaces.transform, glm::radians((float)model.y), glm::vec3(0.0f, -1.0f, 0.0f));
		blockFaces.transform = glm::translate(blockFaces.transform, glm::vec3(-0.5f, 0.0f, -0.5f));
		transform = glm::rotate(transform, glm::radians((float)model.y), glm::vec3(0.0f, -1.0f, 0));
	}

	if (model.x != 0) {
		blockFaces.transform = glm::translate(blockFaces.transform, glm::vec3(0.0f, 0.5f, 0.5f));
		blockFaces.transform = glm::rotate(blockFaces.transform, glm::radians((float)model.x), glm::vec3(1.0f, 0.0f, 0.0f));
		blockFaces.transform = glm::translate(blockFaces.transform, glm::vec3(0.0f, -0.5f, -0.5f));
		transform = glm::rotate(transform, glm::radians((float)model.x), glm::vec3(1.0f, 0.0f, 0));
	}

	for (int i = 0; i < FaceDirection::none; i++) {		
		glm::vec4 vec = transform * glm::vec4(FaceDirectionVector[i].glm(), 1.0f);
		blockFaces.faceDirectionVector[i] = Vector(roundf(vec.x), roundf(vec.y), roundf(vec.z));
	}

	blockIdToBlockFaces.insert(std::make_pair(block, blockFaces));

	return blockIdToBlockFaces.find(block)->second;
}

std::string AssetManager::GetAssetNameByBlockId(BlockId block) {
    for (auto& it : assetIds) {
	BlockId value = it.second;
	value.state = 0;
	if (value == block)
		return it.first;
    }
    return "#NF";
}

Asset *AssetManager::GetAssetPtr(const std::string & assetName) {
	OPTICK_EVENT();
	AssetTreeNode *node;
	if (assetName[0] != '/')
		node = GetAssetByAssetName('/' + assetName);
	else
		node = GetAssetByAssetName(assetName);
	if (!node)
		return nullptr;
	return node->asset.get();
}

AssetTreeNode *AssetManager::GetAssetByAssetName(const std::string & assetName) {
	AssetTreeNode *node = assetTree.get();
	unsigned int pos = 1;
	unsigned int prevPos = 1;
	while (pos < assetName.size()) {
		for (; assetName[pos] != '/' && pos < assetName.size(); pos++);
		std::string dirName = assetName.substr(prevPos, pos - prevPos);
		for (const auto &asset : node->childs) {
			if (asset->name == dirName) {
				node = asset.get();
				break;
			}
		}
		pos++;
		prevPos = pos;
	}
	return node;
}

GLuint AssetManager::GetTextureAtlasId()
{
	return atlas->GetRawTextureId();
}

TextureCoord AssetManager::GetTexture(const std::string &assetName) {
	AssetTreeNode *node = GetAssetByAssetName(assetName);
	if (!node || node->type != AssetTreeNode::ASSET_TEXTURE)
		return GetTexture("/minecraft/textures/error");//TODO: remove recursion
	return atlas->GetTexture(reinterpret_cast<AssetTexture*>(node->asset.get())->id);
}
