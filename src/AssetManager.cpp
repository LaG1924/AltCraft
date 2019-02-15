#include "AssetManager.hpp" 

#include <fstream>
#include <experimental/filesystem>

#include <nlohmann/json.hpp>
#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>
#include <sol.hpp>

#include "Utility.hpp"

namespace fs = std::experimental::filesystem::v1;

const fs::path pathToAssets = "./assets/";
const std::string pathToAssetsList = "./items.json";

std::map<std::string, BlockId> assetIds;
std::map<BlockId, std::string> blockIdToBlockName;
std::unique_ptr<AssetTreeNode> assetTree;
std::unique_ptr<TextureAtlas> atlas;
std::map<BlockId, BlockFaces> blockIdToBlockFaces;
sol::state lua;

void LoadIds();
void LoadAssets();
void LoadTextures();
void LoadScripts();

void WalkDirEntry(const fs::directory_entry &dirEntry, AssetTreeNode *node);
void ParseAsset(AssetTreeNode &node);
void ParseAssetTexture(AssetTreeNode &node);
void ParseAssetBlockModel(AssetTreeNode &node);
void ParseAssetBlockState(AssetTreeNode &node);
void ParseAssetShader(AssetTreeNode &node);
void ParseAssetScript(AssetTreeNode &node);

void ParseBlockModels();

void AssetManager::InitAssetManager()
{
	static bool Initialized = false;
	if (Initialized) {
		LOG(WARNING) << "Trying to init AssetManager twice";
	}
	Initialized = true;
	
	LoadAssets();

	auto parseAssetRecur = [](AssetTreeNode &node) {
		ParseAsset(node);
	};

	RecursiveWalkAsset("/", parseAssetRecur);

	LoadTextures();

	LoadIds();
	ParseBlockModels();
	LoadScripts();
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
	WalkDirEntry(fs::directory_entry(pathToAssets), assetTree.get());
}

void LoadTextures() {
	std::vector<TextureData> textureData;
	size_t id = 0;
	AssetManager::RecursiveWalkAsset("/minecraft/textures/", [&](AssetTreeNode &node) {
		TextureData data;
		AssetTexture *textureAsset = dynamic_cast<AssetTexture*>(node.asset.get());
		if (!textureAsset)
			return;
		data.data = std::move(textureAsset->textureData);
		data.width = textureAsset->realWidth;
		data.height = textureAsset->realHeight;
		textureData.push_back(data);
		textureAsset->id = id++;
	});
	atlas = std::make_unique<TextureAtlas>(textureData);
}

void LoadScripts() {
	lua.open_libraries(sol::lib::base, sol::lib::table);

	LOG(INFO) << "Loading lua-init-scripts";
	std::vector<std::string> loadedScripts;
	std::vector<std::string> failedScripts;

	AssetTreeNode *node = AssetManager::GetAssetByAssetName("/");
	for (auto &it : node->childs) {
		for (auto &child : it->childs) {
			if (child->name == "init") {
				AssetScript *asset = dynamic_cast<AssetScript *>(child->asset.get());
				if (!asset) {
					LOG(ERROR) << "Unrecognised script file /" << it->name;
					continue;
				}
				try {
					lua.script(asset->code);
				}
				catch (sol::error &e) {
					LOG(ERROR) << "LUA init-script " << child->name << " failed: " << e.what();
					failedScripts.push_back(it->name);
					continue;
				}
				loadedScripts.push_back(it->name);
			}
		}
	}

	LOG(INFO) << "Lua loaded: " << loadedScripts.size() << "   failed: " << failedScripts.size();
}

void WalkDirEntry(const fs::directory_entry &dirEntry, AssetTreeNode *node) {
	for (auto &file : fs::directory_iterator(dirEntry)) {
		node->childs.push_back(std::make_unique<AssetTreeNode>());
		AssetTreeNode *fileNode = node->childs.back().get();
		fileNode->parent = node;
		fileNode->name = file.path().stem().string();
		if (fs::is_directory(file)) {
			WalkDirEntry(file, fileNode);
		}
		else {
			size_t fileSize = fs::file_size(file);
			fileNode->data.resize(fileSize);
			FILE *f = fopen(file.path().string().c_str(), "rb");
			fread(fileNode->data.data(), 1, fileSize, f);
			fclose(f);
		}
	}
}

void ParseAsset(AssetTreeNode &node) {
	if (node.data.empty() || node.asset)
		return;

	if (node.parent->name == "block" && node.parent->parent->name == "models") {
		ParseAssetBlockModel(node);
		return;
	}

	if (node.parent->name == "blockstates") {
		ParseAssetBlockState(node);
		return;
	}

	if (node.data[0] == 0x89 && node.data[1] == 'P' && node.data[2] == 'N' && node.data[3] == 'G') {
		ParseAssetTexture(node);
		return;
	}

	if (node.parent->name == "shaders") {
		ParseAssetShader(node);
		return;
	}

	if (node.name == "init") {
		ParseAssetScript(node);
		return;
	}
}

void ParseAssetTexture(AssetTreeNode &node) {
	int w, h, n;
	unsigned char *data = stbi_load_from_memory(node.data.data(),node.data.size(), &w, &h, &n, 4);
	if (data == nullptr) {
		return;
	}

	node.asset = std::make_unique<AssetTexture>();
	AssetTexture *asset = dynamic_cast<AssetTexture*>(node.asset.get());
	size_t dataLen = w * h * 4;
	asset->textureData.resize(dataLen);
	std::memcpy(asset->textureData.data(), data, dataLen);
	asset->realWidth = w;
	asset->realHeight = h;
	bool foundAnimationFile = false;
	for (const auto &it : node.parent->childs)
		if (it->name == node.name + ".png") {
			foundAnimationFile = true;
			break;
		}
	asset->frames = foundAnimationFile ? _max(w, h) / _min(w, h) : 1;


	stbi_image_free(data);

	node.data.clear();
	node.data.shrink_to_fit();
}

void ParseAssetBlockModel(AssetTreeNode &node) {
	nlohmann::json modelData = nlohmann::json::parse(node.data);
	BlockModel model;

	if (node.name == "button") {
		int a = 15;
		a++;
	}

	if (modelData.find("parent") != modelData.end()) {
		std::string parentName = modelData["parent"].get<std::string>();
		parentName = parentName.substr(parentName.find('/') + 1);
		for (auto &it : node.parent->childs) {
			if (it->name == parentName) {
				ParseAsset(*it);
				model = dynamic_cast<AssetBlockModel*>(it->asset.get())->blockModel;
				unsigned char *b = reinterpret_cast<unsigned char*>(&model.IsBlock);
			}
		}
	}

	model.BlockName = node.name;

	if (model.BlockName == "block" || model.BlockName == "cube_mirrored")
		model.IsBlock = true;

	if (model.BlockName == "thin_block" || model.BlockName == "leaves")
		model.IsBlock = false;

	if (modelData.find("ambientocclusion") != modelData.end())
		model.AmbientOcclusion = modelData["ambientocclusion"].get<bool>();

	//models.Display

	if (modelData.find("textures") != modelData.end()) {
		for (nlohmann::json::iterator texture = modelData["textures"].begin(); texture != modelData["textures"].end(); ++texture) {
			model.Textures[texture.key()] = texture.value().get<std::string>();
		}
	}

	if (modelData.find("elements") != modelData.end()) {
		model.Elements.clear();
		for (auto& it : modelData["elements"]) {
			BlockModel::ElementData element;

			auto vec = it["from"];
			Vector from(vec[0].get<int>(), vec[1].get<int>(), vec[2].get<int>());
			vec = it["to"];
			Vector to(vec[0].get<int>(), vec[1].get<int>(), vec[2].get<int>());

			element.from = from;
			element.to = to;

			if (it.find("rotation") != it.end()) {
				vec = it["rotation"]["origin"];
				Vector rotOrig(vec[0].get<int>(), vec[1].get<int>(), vec[2].get<int>());

				element.rotationOrigin = rotOrig;
				element.rotationAxis = (it["rotation"]["axis"].get<std::string>() == "x") ? BlockModel::ElementData::Axis::x : ((it["rotation"]["axis"].get<std::string>() == "y") ? BlockModel::ElementData::Axis::y : BlockModel::ElementData::Axis::z);
				if (it["rotation"].find("angle") != it["rotation"].end())
					element.rotationAngle = it["rotation"]["angle"].get<int>();

				if (it["rotation"].find("rescale") != it["rotation"].end())
					element.rotationRescale = it["rotation"]["rescale"].get<bool>();
			}

			if (it.find("shade") != it.end())
				element.shade = it["shade"].get<bool>();

			for (nlohmann::json::iterator faceIt = it["faces"].begin(); faceIt != it["faces"].end(); ++faceIt) {
				auto face = faceIt.value();
				BlockModel::ElementData::FaceData faceData;

				FaceDirection faceDir;
				if (faceIt.key() == "down")
					faceDir = FaceDirection::down;
				else if (faceIt.key() == "up")
					faceDir = FaceDirection::up;
				else if (faceIt.key() == "north")
					faceDir = FaceDirection::north;
				else if (faceIt.key() == "south")
					faceDir = FaceDirection::south;
				else if (faceIt.key() == "west")
					faceDir = FaceDirection::west;
				else if (faceIt.key() == "east")
					faceDir = FaceDirection::east;

				if (face.find("uv") != face.end()) {
					BlockModel::ElementData::FaceData::Uv uv;
					uv.x1 = face["uv"][0];
					uv.y1 = face["uv"][1];
					uv.x2 = face["uv"][2];
					uv.y2 = face["uv"][3];
					faceData.uv = uv;
				}

				FaceDirection cullface = FaceDirection::none;
				if (face.find("cullface") != face.end()) {
					if (face["cullface"] == "down")
						cullface = FaceDirection::down;
					else if (face["cullface"] == "up")
						cullface = FaceDirection::up;
					else if (face["cullface"] == "north")
						cullface = FaceDirection::north;
					else if (face["cullface"] == "south")
						cullface = FaceDirection::south;
					else if (face["cullface"] == "west")
						cullface = FaceDirection::west;
					else if (face["cullface"] == "east")
						cullface = FaceDirection::east;
				}
				faceData.cullface = cullface;

				faceData.texture = face["texture"].get<std::string>();

				if (face.find("rotation") != face.end())
					faceData.rotation = face["rotation"].get<int>();

				if (face.find("tintindex") != face.end())
					faceData.tintIndex = true;

				element.faces[faceDir] = faceData;
			}

			model.Elements.push_back(element);
		}
	}

	node.asset = std::make_unique<AssetBlockModel>();
	dynamic_cast<AssetBlockModel*>(node.asset.get())->blockModel = model;
	node.data.clear();
	node.data.shrink_to_fit();
}

void ParseAssetBlockState(AssetTreeNode &node) {
	nlohmann::json j = nlohmann::json::parse(node.data);

	BlockState blockState;
	if (j.find("multipart") != j.end())
		return;

	j = j["variants"];
	for (auto variantIt = j.begin(); variantIt != j.end(); variantIt++) {
		std::string variantName = variantIt.key();
		BlockStateVariant variant;
		variant.variantName = variantName;
		if (variantIt.value().is_array()) {
			for (auto &it : variantIt.value()) {
				BlockStateVariant::Model model;
				model.modelName = it["model"].get<std::string>();
				if (it.count("x"))
					model.x = it["x"].get<int>();
				if (it.count("y"))
					model.y = it["y"].get<int>();
				if (it.count("uvlock"))
					model.uvLock = it["uvlock"].get<int>();
				if (it.count("weight"))
					model.weight = it["weight"].get<int>();
				variant.models.push_back(model);
			}
		} else {
			BlockStateVariant::Model model;
			model.modelName = variantIt.value()["model"].get<std::string>();
			if (variantIt.value().count("x"))
				model.x = variantIt.value()["x"].get<int>();
			if (variantIt.value().count("y"))
				model.y = variantIt.value()["y"].get<int>();
			if (variantIt.value().count("uvlock"))
				model.uvLock = variantIt.value()["uvlock"].get<int>();
			if (variantIt.value().count("weight"))
				model.weight = variantIt.value()["weight"].get<int>();
			variant.models.push_back(model);
		}
		blockState.variants[variant.variantName] = variant;
	}

	node.asset = std::make_unique<AssetBlockState>();
	AssetBlockState *asset = dynamic_cast<AssetBlockState*>(node.asset.get());
	asset->blockState = blockState;

	node.data.clear();
	node.data.shrink_to_fit();
}

void ParseAssetShader(AssetTreeNode &node) {
	try {
		nlohmann::json j = nlohmann::json::parse(node.data);

		std::string vertPath = j["vert"].get<std::string>();
		std::string fragPath = j["frag"].get<std::string>();

		AssetTreeNode *vertAsset = AssetManager::GetAssetByAssetName(vertPath);
		AssetTreeNode *fragAsset = AssetManager::GetAssetByAssetName(fragPath);
		std::string vertSource((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());
		std::string fragSource((char*)fragAsset->data.data(), (char*)fragAsset->data.data() + fragAsset->data.size());

		std::vector<std::string> uniforms;

		for (auto &it : j["uniforms"]) {
			uniforms.push_back(it.get<std::string>());
		}

		node.asset = std::make_unique<AssetShader>();
		AssetShader *asset = dynamic_cast<AssetShader*>(node.asset.get());
		asset->shader = std::make_unique<Shader>(vertSource, fragSource, uniforms);
	} catch (...) {
		glCheckError();
		return;
	}
}

void ParseAssetScript(AssetTreeNode &node) {
	node.asset = std::make_unique<AssetScript>();
	AssetScript *asset = dynamic_cast<AssetScript*>(node.asset.get());
	asset->code = std::string((char*)node.data.data(), (char*)node.data.data() + node.data.size());
	node.data.clear();
	node.data.shrink_to_fit();
}

void ParseBlockModels() {
	std::string textureName;

	auto parseBlockModel = [&](AssetTreeNode &node) {
		if (!node.asset)
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
					textureName.insert(0, "minecraft/textures/");
					AssetTexture *assetTexture = AssetManager::GetAsset<AssetTexture>(textureName);
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

	AssetManager::RecursiveWalkAsset("/minecraft/models/", parseBlockModel);
}

BlockFaces &AssetManager::GetBlockModelByBlockId(BlockId block) {
	auto it = blockIdToBlockFaces.find(block);
	if (it != blockIdToBlockFaces.end())
		return it->second;

	if (block.id == 7788) {
		BlockFaces blockFaces;
		blockFaces.transform = glm::mat4(1.0);
		blockFaces.faces = GetAsset<AssetBlockModel>("/minecraft/models/block/error")->blockModel.parsedFaces;
		blockFaces.isBlock = GetAsset<AssetBlockModel>("/minecraft/models/block/error")->blockModel.IsBlock;
		for (int i = 0; i < FaceDirection::none; i++) {
			blockFaces.faceDirectionVector[i] = FaceDirectionVector[i];
		}
		blockIdToBlockFaces.insert(std::make_pair(block, blockFaces));
		return blockIdToBlockFaces.find(block)->second;
	}

	auto blockStateName = TransformBlockIdToBlockStateName(block);
	AssetBlockState *asset = GetAsset<AssetBlockState>("/minecraft/blockstates/" + blockStateName.first);
	if (!asset)
		return GetBlockModelByBlockId(BlockId{ 7788,0 });
	
	BlockState &blockState = asset->blockState;
	if (blockState.variants.find(blockStateName.second) == blockState.variants.end())
		return GetBlockModelByBlockId(BlockId{ 7788,0 });

	BlockStateVariant &variant = blockState.variants[blockStateName.second];
	if (variant.models.empty())
		return GetBlockModelByBlockId(BlockId{ 7788,0 });

	BlockStateVariant::Model &model = variant.models[0];
	AssetBlockModel *assetModel = GetAsset<AssetBlockModel>("/minecraft/models/block/" + model.modelName);
	if (!assetModel)
		return GetBlockModelByBlockId(BlockId{ 7788,0 });
	
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
	AssetTreeNode *node;
	if (assetName[0] != '/')
		node = GetAssetByAssetName('/' + assetName);
	else
		node = GetAssetByAssetName(assetName);
	if (!node)
		return nullptr;
	return node->asset.get();
}

void AssetManager::RecursiveWalkAsset(const std::string & assetPath, std::function<void(AssetTreeNode&)> fnc) {
	AssetTreeNode *assetNode = GetAssetByAssetName(assetPath);
	
	std::function<void(AssetTreeNode&)> walkAssetRecur = [&](AssetTreeNode &node) {
		fnc(node);
		for (auto& it : node.childs) {
			walkAssetRecur(*it.get());
		}
	};

	walkAssetRecur(*assetNode);
}

AssetTreeNode *AssetManager::GetAssetByAssetName(const std::string & assetName) {
	AssetTreeNode *node = assetTree.get();
	unsigned int pos = 1;
	unsigned int prevPos = 1;
	size_t x = assetName.size();
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

TextureCoord AssetManager::GetTexture(const std::string assetName) {
	AssetTexture *asset = GetAsset<AssetTexture>(assetName);
	if (!asset)
		return GetTexture("/minecraft/textures/error");
	return atlas->GetTexture(asset->id);
}
