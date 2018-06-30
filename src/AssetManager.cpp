#include "AssetManager.hpp" 

#include <fstream>
#include <experimental/filesystem>

#include <nlohmann/json.hpp>
#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.hpp"

namespace fs = std::experimental::filesystem::v1;

const fs::path pathToAssets = "./assets/";
//const fs::path pathToAssetsList = "./items.json";
//const fs::path pathToTextureIndex = "./textures.json";
const std::string pathToAssetsList = "./items.json";
const std::string pathToTextureIndex = "./textures.json";

const fs::path pathToModels  = "./assets/minecraft/models/";

AssetManager::AssetManager() {
	LoadAssets();
    LoadIds();
    LoadTextureResources();
    LoadBlockModels();
	ParseBlockModels();	
}

void AssetManager::LoadIds() {
    std::ifstream in(pathToAssetsList);
    nlohmann::json index;
    in >> index;
    for (auto &it:index) {
        unsigned short id = it["type"].get<int>();
        unsigned char state = it["meta"].get<int>();
        std::string blockName = it["text_type"].get<std::string>();
        assetIds[blockName] = BlockId{ id, state };
    }
    LOG(INFO) << "Loaded " << assetIds.size() << " ids";
}

AssetManager::~AssetManager() {
    delete textureAtlas;
}

//TODO: This function must be replaced with runtime texture atlas generating
void AssetManager::LoadTextureResources() {
    std::ifstream in(pathToTextureIndex);
    nlohmann::json index;
    in >> index;
    std::string filename = index["meta"]["image"].get<std::string>();
    float textureWidth = index["meta"]["size"]["w"].get<int>();
    float textureHeight = index["meta"]["size"]["h"].get<int>();
    size_t sizeName = 0,sizeTexture = 0;
    for (auto &it:index["frames"]) {
        auto frame = it["frame"];
        TextureCoordinates coord;
        coord.x = frame["x"].get<int>() / textureWidth;
        coord.y = frame["y"].get<int>() / textureHeight;
        coord.w = frame["w"].get<int>() / textureWidth;
        coord.h = frame["h"].get<int>() / textureHeight;
        std::string assetName = it["filename"].get<std::string>();
        assetName.insert(0, "minecraft/textures/");
        assetName.erase(assetName.length() - 4);
        assetName.shrink_to_fit();
        sizeName += sizeof(assetName) + assetName.capacity();
        sizeTexture += sizeof(coord);
        assetTextures[assetName] = coord;
    }
    textureAtlas = new Texture(filename);
    LOG(INFO) << "Texture atlas id is " << textureAtlas->texture;
}

TextureCoordinates AssetManager::GetTextureByAssetName(const std::string &AssetName) {
	auto it = assetTextures.find(AssetName);
	if (it != assetTextures.end())
		return it->second;
    else
        return TextureCoordinates{-1, -1, -1, -1};
}

std::string AssetManager::GetTextureAssetNameByBlockId(BlockTextureId block) {
    //Block sides: 0 - bottom, 1 - top, 2 - north, 3 - south, 4 - west, 5 - east 6 - every side
    const std::map<BlockTextureId, std::string> lookupTable = {
            {BlockTextureId(0, 0),     "minecraft/textures/blocks/air"},
            {BlockTextureId(1, 0),     "minecraft/textures/blocks/stone"},
            {BlockTextureId(1, 1),     "minecraft/textures/blocks/stone_granite"},

            {BlockTextureId(2, 0, 0),  "minecraft/textures/blocks/dirt"},
            {BlockTextureId(2, 0, 1),  "minecraft/textures/blocks/grass_top"},
            {BlockTextureId(2, 0, 2),  "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 3),  "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 4),  "minecraft/textures/blocks/grass_side"},
            {BlockTextureId(2, 0, 5),  "minecraft/textures/blocks/grass_side"},

            {BlockTextureId(3, 0),     "minecraft/textures/blocks/dirt"},
            {BlockTextureId(4, 0),     "minecraft/textures/blocks/cobblestone"},
            {BlockTextureId(5, 0),     "minecraft/textures/blocks/planks"},

            {BlockTextureId(7, 0),     "minecraft/textures/blocks/bedrock"},

            {BlockTextureId(17, 0, 0), "minecraft/textures/blocks/log_oak_top"},
            {BlockTextureId(17, 0, 1), "minecraft/textures/blocks/log_oak_top"},
            {BlockTextureId(17, 0, 2), "minecraft/textures/blocks/log_oak"},
            {BlockTextureId(17, 0, 3), "minecraft/textures/blocks/log_oak"},
            {BlockTextureId(17, 0, 4), "minecraft/textures/blocks/log_oak"},
            {BlockTextureId(17, 0, 5), "minecraft/textures/blocks/log_oak"},

            {BlockTextureId(17, 1, 0), "minecraft/textures/blocks/log_spruce_top"},
            {BlockTextureId(17, 1, 1), "minecraft/textures/blocks/log_spruce_top"},
            {BlockTextureId(17, 1, 2), "minecraft/textures/blocks/log_spruce"},
            {BlockTextureId(17, 1, 3), "minecraft/textures/blocks/log_spruce"},
            {BlockTextureId(17, 1, 4), "minecraft/textures/blocks/log_spruce"},
            {BlockTextureId(17, 1, 5), "minecraft/textures/blocks/log_spruce"},

            {BlockTextureId(17, 2, 0), "minecraft/textures/blocks/log_birch_top"},
            {BlockTextureId(17, 2, 1), "minecraft/textures/blocks/log_birch_top"},
            {BlockTextureId(17, 2, 2), "minecraft/textures/blocks/log_birch"},
            {BlockTextureId(17, 2, 3), "minecraft/textures/blocks/log_birch"},
            {BlockTextureId(17, 2, 4), "minecraft/textures/blocks/log_birch"},
            {BlockTextureId(17, 2, 5), "minecraft/textures/blocks/log_birch"},

            {BlockTextureId(17, 3, 0), "minecraft/textures/blocks/log_jungle_top"},
            {BlockTextureId(17, 3, 1), "minecraft/textures/blocks/log_jungle_top"},
            {BlockTextureId(17, 3, 2), "minecraft/textures/blocks/log_jungle"},
            {BlockTextureId(17, 3, 3), "minecraft/textures/blocks/log_jungle"},
            {BlockTextureId(17, 3, 4), "minecraft/textures/blocks/log_jungle"},
            {BlockTextureId(17, 3, 5), "minecraft/textures/blocks/log_jungle"},

            {BlockTextureId(18, 0),    "minecraft/textures/blocks/leaves_oak"},
            {BlockTextureId(18, 1),    "minecraft/textures/blocks/leaves_spruce"},
            {BlockTextureId(18, 2),    "minecraft/textures/blocks/leaves_birch"},
            {BlockTextureId(18, 3),    "minecraft/textures/blocks/leaves_jungle"},

            {BlockTextureId(61, 0, 0), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(61, 0, 1), "minecraft/textures/blocks/furnace_top"},
            {BlockTextureId(61, 0, 2), "minecraft/textures/blocks/furnace_front_off"},
            {BlockTextureId(61, 0, 3), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(61, 0, 4), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(61, 0, 5), "minecraft/textures/blocks/furnace_side"},

            {BlockTextureId(62, 0, 0), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(62, 0, 1), "minecraft/textures/blocks/furnace_top"},
            {BlockTextureId(62, 0, 2), "minecraft/textures/blocks/furnace_front_on"},
            {BlockTextureId(62, 0, 3), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(62, 0, 4), "minecraft/textures/blocks/furnace_side"},
            {BlockTextureId(62, 0, 5), "minecraft/textures/blocks/furnace_side"},


            {BlockTextureId(31, 0),    "minecraft/textures/blocks/deadbush"},
            {BlockTextureId(31, 1),    "minecraft/textures/blocks/tallgrass"},
            {BlockTextureId(31, 2),    "minecraft/textures/blocks/fern"},
    };

    auto ret = lookupTable.find(block);
    return (ret == lookupTable.end()) ? "" : ret->second;
}

GLuint AssetManager::GetTextureAtlas() {
    return textureAtlas->texture;    
}

TextureCoordinates AssetManager::GetTextureByBlock(BlockTextureId block) {
    std::string assetName = this->GetTextureAssetNameByBlockId(block);
    return this->GetTextureByAssetName(assetName);
}

const std::map<BlockTextureId, glm::vec4> &AssetManager::GetTextureAtlasIndexes() {
    if (!textureAtlasIndexes.empty())
        return textureAtlasIndexes;

    LOG(INFO) << "Initializing texture atlas...";
    for (int id = 1; id < 128; id++) {
        for (int state = 0; state < 16; state++) {
            BlockTextureId blockTextureId(id, state, 6);
            if (!this->GetTextureByBlock(blockTextureId) &&
                !this->GetTextureByBlock(BlockTextureId(id, state, 0))) {
                continue;
            }

            if (this->GetTextureByBlock(blockTextureId)) {
                for (int i = 0; i < 6; i++) {
                    TextureCoordinates tc = this->GetTextureByBlock(BlockTextureId(id, state, 6));
                    textureAtlasIndexes[BlockTextureId(id, state, i)] = glm::vec4(tc.x, tc.y, tc.w, tc.h);
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    TextureCoordinates tc = this->GetTextureByBlock(BlockTextureId(id, state, i));
                    textureAtlasIndexes[BlockTextureId(id, state, i)] = glm::vec4(tc.x, tc.y, tc.w, tc.h);
                }
            }
        }
    }
    LOG(INFO) << "Created " << textureAtlasIndexes.size() << " texture indexes";

    return textureAtlasIndexes;
}

AssetManager &AssetManager::Instance() {
    static AssetManager assetManager;
    return assetManager;
}

const BlockModel *AssetManager::GetBlockModelByBlockId(BlockId block) {
    block.state = 0;
    if (blockIdToBlockName.find(block) == blockIdToBlockName.end()) {
        std::string blockName = "";
        for (const auto& it : assetIds) {
            if (BlockId{ it.second.id,0 } == block) {
                blockName = it.first;
                break;
            }
        }
        if (blockName == "grass")
            blockName = "grass_normal";
        if (blockName == "torch")
            blockName = "normal_torch";
        if (blockName == "leaves")
            blockName = "oak_leaves";
        if (blockName == "tallgrass")
            blockName = "tall_grass";
        if (blockName == "log")
            blockName = "oak_bark";
        if (blockName == "snow_layer")
            blockName = "snow_height2";

        blockName = "block/" + blockName;

        if (blockName == "")
            return nullptr;

        blockIdToBlockName[block] = blockName;
    }

    std::string blockName = blockIdToBlockName[block];

    auto modelIt = models.find(blockName);
	return (modelIt == models.end()) ? GetBlockModelByBlockId(BlockId{ 57,0 }) : &modelIt->second;
}

void AssetManager::LoadBlockModels() {
    std::function<void(std::string)> parseModel = [&](std::string ModelName) {
        if (models.find(ModelName) != models.end())
            return;

        fs::path ModelPath = pathToModels / fs::path(ModelName + ".json");
        std::ifstream in(ModelPath);
        if (!in.is_open())
            throw std::runtime_error("Trying to load unknown model \"" + ModelName + "\" at " + ModelPath.generic_string());
        nlohmann::json modelData;
        in >> modelData;
        BlockModel model;        

        if (modelData.find("parent") != modelData.end()) {
            if (models.find(modelData["parent"]) == models.end())
                parseModel(modelData["parent"].get<std::string>());

            model = models.find(modelData["parent"])->second;
        }

        model.BlockName = ModelName;

        if (model.BlockName == "block/block")
            model.IsBlock = true;

        if (model.BlockName == "block/thin_block" || model.BlockName=="block/leaves")
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
                Vector from (vec[0].get<int>(), vec[1].get<int>(), vec[2].get<int>());
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

                    BlockModel::ElementData::FaceDirection faceDir;
                    if (faceIt.key() == "down")
                        faceDir = BlockModel::ElementData::FaceDirection::down;
                    else if (faceIt.key() == "up")
                        faceDir = BlockModel::ElementData::FaceDirection::up;
                    else if (faceIt.key() == "north")
                        faceDir = BlockModel::ElementData::FaceDirection::north;
                    else if (faceIt.key() == "south")
                        faceDir = BlockModel::ElementData::FaceDirection::south;
                    else if (faceIt.key() == "west")
                        faceDir = BlockModel::ElementData::FaceDirection::west;
                    else if (faceIt.key() == "east")
                        faceDir = BlockModel::ElementData::FaceDirection::east;
                                        
                    if (face.find("uv") != face.end()) {
                        BlockModel::ElementData::FaceData::Uv uv;
                        uv.x1 = face["uv"][0];
                        uv.y1 = face["uv"][1];
                        uv.x2 = face["uv"][2];
                        uv.y2 = face["uv"][3];
                        faceData.uv = uv;
                    }

					BlockModel::ElementData::FaceDirection cullface = BlockModel::ElementData::FaceDirection::none;
                    if (face.find("cullface") != face.end()) {
                        if (face["cullface"] == "down")
                            cullface = BlockModel::ElementData::FaceDirection::down;
                        else if (face["cullface"] == "up")
                            cullface = BlockModel::ElementData::FaceDirection::up;
                        else if (face["cullface"] == "north")
                            cullface = BlockModel::ElementData::FaceDirection::north;
                        else if (face["cullface"] == "south")
                            cullface = BlockModel::ElementData::FaceDirection::south;
                        else if (face["cullface"] == "west")
                            cullface = BlockModel::ElementData::FaceDirection::west;
                        else if (face["cullface"] == "east")
                            cullface = BlockModel::ElementData::FaceDirection::east;                        
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

        models.insert(std::make_pair(ModelName, model));
    };

    fs::path pathToBlockModels = pathToModels.generic_string() + "/block/";

    for (auto& dirEntry : fs::recursive_directory_iterator(pathToBlockModels)) {
        if (dirEntry.path().extension() != ".json")
            continue;
        
        std::string modelName = dirEntry.path().stem().generic_string();
        parseModel("block/" + modelName);
    }	
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

void AssetManager::ParseBlockModels() {
	std::string textureName;

	for (auto &modelIt : models) {
		const auto &modelName = modelIt.first;
		auto &model = modelIt.second;
		
		for (const auto& element : model.Elements) {
			Vector t = element.to - element.from;
			VectorF elementSize(VectorF(t.x, t.y, t.z) / 16.0f);
			VectorF elementOrigin(VectorF(element.from.x, element.from.y, element.from.z) / 16.0f);			

			glm::mat4 elementTransform;

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

				glm::mat4 rotationMat;
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
				BlockModel::ParsedFace parsedFace;
				parsedFace.visibility = face.second.cullface;

				glm::mat4 faceTransform;
				switch (face.first) {
				case BlockModel::ElementData::FaceDirection::down:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0, 0));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					break;
				case BlockModel::ElementData::FaceDirection::up:
					faceTransform = glm::translate(elementTransform, glm::vec3(0.0f, 1.0f, 0.0f));
					break;
				case BlockModel::ElementData::FaceDirection::north:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 1));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1, 0, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1.0f));
					break;
				case BlockModel::ElementData::FaceDirection::south:
					faceTransform = glm::translate(elementTransform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
					break;
				case BlockModel::ElementData::FaceDirection::west:
					faceTransform = glm::translate(elementTransform, glm::vec3(1, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
					break;
				case BlockModel::ElementData::FaceDirection::east:
					faceTransform = glm::translate(elementTransform, glm::vec3(0, 0, 0));
					faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
					break;
				}
				parsedFace.transform = faceTransform;
				glm::vec4 texture;
				textureName = face.second.texture;
				if (model.Textures.empty()) {
					texture = GetTextureByAssetName("minecraft/texture/blocks/tnt_side");
				}
				else {
					while (textureName[0] == '#') {
						textureName.erase(0, 1);
						auto textureIt = model.Textures.find(textureName);
						textureName = textureIt != model.Textures.end() ? textureIt->second : "minecraft/texture/blocks/tnt_side";
					}
					textureName.insert(0, "minecraft/textures/");
					texture = GetTextureByAssetName(textureName);

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
						double W = texture.z;
						double H = texture.w;

						texture = glm::vec4{ X + x * W, Y + y * H, w * W , h * H };
					}
				}
				parsedFace.texture = texture;
				if (face.second.tintIndex)
					parsedFace.color = glm::vec3(0.275, 0.63, 0.1);
				else
					parsedFace.color = glm::vec3(0, 0, 0);

				model.parsedFaces.push_back(parsedFace);
			}
		}
	}
}

std::unique_ptr<Asset> ParseAsset(const fs::path &file) {
	return std::unique_ptr<Asset>();
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
			fileNode->asset = ParseAsset(file);
		}
	}
}

void AssetManager::LoadAssets() {
	assetTree = std::make_unique<AssetTreeNode>();
	assetTree->name = "/";
	WalkDirEntry(fs::directory_entry(pathToAssets), assetTree.get());
}
