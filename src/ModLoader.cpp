#include "ModLoader.hpp"
#include "Utility.hpp"

#include <easylogging++.h>
#include <nlohmann/json.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

std::vector<std::shared_ptr<ModLoader::Mod>> mods;

void ModLoader::LoadMod(AssetTreeNode &node) {
	for (auto& it : node.childs) {
		if		(it->name=="code")
			LoadCode(*it);

		else if	(it->name=="shaders")
			RecursiveWalkAssetFiles(*it.get(), ParseAssetShader);

		else if	(it->name=="blockstates")
			RecursiveWalkAssetFiles(*it.get(), ParseAssetBlockState);

		else if	(it->name=="models")
			LoadModels(*it.get());

		else if	(it->name=="textures")
			RecursiveWalkAssetFiles(*it.get(), ParseAssetTexture);

		else if (it->name=="acmod")
			LoadModinfo(*it.get());

		else if (it->name=="pack")
			try{
				LoadMcmeta(*it.get());
			}catch(nlohmann::json::type_error e){
				LOG(ERROR) << e.what();
			}

		else
			LOG(WARNING) << "Unknown asset type \"" << it->name << "\" from " << node.name;
	}
}

void ModLoader::LoadModels(AssetTreeNode &node){
	for (auto& it : node.childs) {
		if		(it->name=="block")
			RecursiveWalkAssetFiles(*it.get(), ParseAssetBlockModel);

		else
			LOG(WARNING)<<"Unknown model type \"" << it->name << "\" from " << node.parent->name;
	}
}
void ModLoader::LoadCode(AssetTreeNode &node){
	for (auto& it : node.childs) {
		if	      (it->name=="lua"){
			RecursiveWalkAssetFiles(*it, ParseAssetLua);
		}else
			LOG(WARNING) << "Unknown code type \"" << it->name << "\" from " << node.name;
	}
}

void ModLoader::LoadModinfo(AssetTreeNode &node){
	//Content of modinfo
	//modid - internal mod name
	//name - displayable mod name
	//version
	//description
	//type {resourcepack, lua}
	//authors
	//url
	//updateUrl
	nlohmann::json modinfo = nlohmann::json::parse(node.data);
	std::shared_ptr<Mod> mod=std::make_shared<Mod>();

	if (modinfo.find("modid") != modinfo.end())
		mod->modid=modinfo["modid"];
	else
		mod->modid=node.parent->name;

	mod->dirname=node.parent->name;

	std::string type=modinfo["type"];
	if		(type == "resourcepack")
		mod->type=Mod::resourcepack;
	else if	(type == "lua")
		mod->type=Mod::lua;
	else{
		LOG(FATAL) << "Unsopported mod type for " << node.name;
		mod->type=Mod::resourcepack;
	}

	if (modinfo.find("name") != modinfo.end())
		mod->name=modinfo["name"];

	if (modinfo.find("version") != modinfo.end())
		mod->version=modinfo["version"];

	if (modinfo.find("description") != modinfo.end())
		mod->description=modinfo["description"];

	if (modinfo.find("url") != modinfo.end())
		mod->url=modinfo["url"];

	if (modinfo.find("authors") != modinfo.end()) {
		for (auto& it : modinfo["authors"]) {
			mod->authors.push_back(it.get<std::string>());
		}
	}

	LOG(INFO) << (mod->name.empty() ? mod->modid : mod->name) << " module loaded";

	mods.push_back(mod);
}

void ModLoader::LoadMcmeta(AssetTreeNode &node){
	nlohmann::json mcmeta = nlohmann::json::parse(node.data);
	auto pack = mcmeta["pack"];
	if(pack["pack_format"] != 3)
		return;

	std::shared_ptr<Mod> existing;
	existing = GetModByDirName(node.parent->name);
	if (existing) {
		if (existing->description.empty())
			existing->description = pack["desctiption"];
	} else {
		existing->modid = node.parent->name;
		existing->dirname = node.parent->name;
		existing->type = Mod::resourcepack;
		existing->description = pack["description"];
		mods.push_back(existing);
	}
}

void ModLoader::ParseAssetTexture(AssetTreeNode &node) {
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

void ModLoader::ParseAssetBlockModel(AssetTreeNode &node) {
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
				if(!(it->data.empty() || it->asset))
					ParseAssetBlockModel(*it);
				model = dynamic_cast<AssetBlockModel*>(it->asset.get())->blockModel;
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

void ModLoader::ParseAssetBlockState(AssetTreeNode &node) {
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

void ModLoader::ParseAssetShader(AssetTreeNode &node) {
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

void ModLoader::ParseAssetLua(AssetTreeNode &node) {
	node.asset = std::make_unique<AssetScript>();
	AssetScript *asset = dynamic_cast<AssetScript*>(node.asset.get());
	asset->code = std::string((char*)node.data.data(), (char*)node.data.data() + node.data.size());
	node.data.clear();
	node.data.shrink_to_fit();
}


void ModLoader::WalkDirEntry(const fs::directory_entry &dirEntry, AssetTreeNode *node) {
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


void ModLoader::RecursiveWalkAssetFiles(AssetTreeNode &assetNode, std::function<void(AssetTreeNode&)> fnc) {

	std::function<void(AssetTreeNode&)> walkAssetRecur = [&](AssetTreeNode &node) {
		for (auto& it : node.childs) {
			if(it->data.empty() || it->asset)
				walkAssetRecur(*it.get());
			else
				fnc(*it.get());
		}
	};

	walkAssetRecur(assetNode);
}

void ModLoader::RecursiveWalkAssetPath(const std::string & assetPath, std::function<void(AssetTreeNode&)> fnc) {
	AssetTreeNode *assetNode = AssetManager::GetAssetByAssetName(assetPath);

	std::function<void(AssetTreeNode&)> walkAssetRecur = [&](AssetTreeNode &node) {
		fnc(node);
		for (auto& it : node.childs) {
			walkAssetRecur(*it.get());
		}
	};

	walkAssetRecur(*assetNode);
}

std::shared_ptr<ModLoader::Mod> ModLoader::GetModByModid(const std::string &modid){
	for (auto& it : mods) {
		if (modid == it->modid)
			return it;
	}
	return std::shared_ptr<Mod>(nullptr);
}

std::shared_ptr<ModLoader::Mod> ModLoader::GetModByDirName(const std::string &dirname){
	for (auto& it : mods) {
		if (dirname == it->dirname)
			return it;
	}
	return std::shared_ptr<Mod>(nullptr);
}
