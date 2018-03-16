#include "RendererSectionData.hpp"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "World.hpp"
#include "AssetManager.hpp"

inline const BlockId& GetBlockId(const Vector& pos, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[pos.y * 256 + pos.z * 16 + pos.x];
}

inline const BlockId& GetBlockId(int x, int y, int z, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[y * 256 + z * 16 + x];
}

void AddFacesByBlockModel(Vector blockPos, const BlockModel &model, glm::mat4 transform, unsigned char light, unsigned char skyLight, const std::array<unsigned char, 16 * 16 * 16>& visibility, std::string &textureName, RendererSectionData &data) {
    glm::mat4 elementTransform, faceTransform;
    for (const auto& element : model.Elements) {
        Vector t = element.to - element.from;
        VectorF elementSize(VectorF(t.x, t.y, t.z) / 16.0f);
        VectorF elementOrigin(VectorF(element.from.x, element.from.y, element.from.z) / 16.0f);
        elementTransform = transform;

        /*if (element.rotationAngle != 0) {
        const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
        const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
        const glm::vec3 zAxis(0.0f, 0.0f, 1.0f);
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
        elementTransform = glm::translate(elementTransform, -rotateOrigin.glm());
        elementTransform = glm::rotate(elementTransform, glm::radians(float(45)), yAxis);
        elementTransform = glm::translate(elementTransform, rotateOrigin.glm());
        }*/
        elementTransform = glm::translate(elementTransform, elementOrigin.glm());
        elementTransform = glm::scale(elementTransform, elementSize.glm());

        for (const auto& face : element.faces) {
            if (face.second.cullface != BlockModel::ElementData::FaceDirection::none) {
                unsigned char visible = visibility[blockPos.y * 256 + blockPos.z * 16 + blockPos.x];

                switch (face.second.cullface) {
                case BlockModel::ElementData::FaceDirection::down:
                    if (visible >> 0 & 0x1)
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::up:
                    if (visible >> 1 & 0x1)
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::north:
                    if (visible >> 2 & 0x1)
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::south:
                    if (visible >> 3 & 0x1)
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::west:
                    if (visible >> 4 & 0x1)
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::east:
                    if (visible >> 5 & 0x1)
                        continue;
                    break;
                }
            }

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
            data.models.push_back(faceTransform);
            textureName = face.second.texture;
            while (textureName[0] == '#') {
				textureName.erase(0, 1);
                textureName = model.Textures.find(textureName)->second;
            }
			textureName.insert(0, "minecraft/textures/");
            glm::vec4 texture = AssetManager::Instance().GetTextureByAssetName(textureName);

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
			data.textures.push_back(texture);
            if (face.second.tintIndex)
				data.colors.push_back(glm::vec3(0.275, 0.63, 0.1));
            else
				data.colors.push_back(glm::vec3(0, 0, 0));
			data.lights.push_back(glm::vec2(light, skyLight));
        }
    }
}

const BlockModel* GetInternalBlockModel(const BlockId& id, std::vector<std::pair<BlockId, const BlockModel *>> &idModels) {
    for (const auto& it : idModels) {
        if (it.first == id)
            return it.second;
    }
    idModels.push_back(std::make_pair(id, AssetManager::Instance().GetBlockModelByBlockId(id)));
    return idModels.back().second;
}

std::array<unsigned char, 4096> GetBlockVisibilityData(const SectionsData &sections, const std::array<BlockId, 4096> &blockIdData, std::vector<std::pair<BlockId, const BlockModel *>> &idModels) {
	std::array<unsigned char, 4096> arr;
	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				unsigned char value = 0;
				BlockId blockIdDown;
				BlockId blockIdUp;
				BlockId blockIdNorth;
				BlockId blockIdSouth;
				BlockId blockIdWest;
				BlockId blockIdEast;

				switch (y) {
				case 0:
					blockIdDown = sections.bottom.GetBlockId(Vector(x, 15, z));
					blockIdUp = GetBlockId(x, 1, z, blockIdData);
					break;
				case 15:
					blockIdDown = GetBlockId(x, 14, z, blockIdData);
					blockIdUp = sections.top.GetBlockId(Vector(x, 0, z));
					break;
				default:
					blockIdDown = GetBlockId(x, y - 1, z, blockIdData);
					blockIdUp = GetBlockId(x, y + 1, z, blockIdData);
					break;
				}

				switch (z) {
				case 0:
					blockIdNorth = GetBlockId(x, y, 1, blockIdData);
					blockIdSouth = sections.south.GetBlockId(Vector(x, y, 15));
					break;
				case 15:
					blockIdNorth = sections.north.GetBlockId(Vector(x, y, 0));
					blockIdSouth = GetBlockId(x, y, 14, blockIdData);
					break;
				default:
					blockIdNorth = GetBlockId(x, y, z + 1, blockIdData);
					blockIdSouth = GetBlockId(x, y, z - 1, blockIdData);
					break;
				}

				switch (x) {
				case 0:
					blockIdWest = GetBlockId(1, y, z, blockIdData);
					blockIdEast = sections.east.GetBlockId(Vector(15, y, z));
					break;
				case 15:
					blockIdWest = sections.west.GetBlockId(Vector(0, y, z));
					blockIdEast = GetBlockId(14, y, z, blockIdData);
					break;
				default:
					blockIdWest = GetBlockId(x + 1, y, z, blockIdData);
					blockIdEast = GetBlockId(x - 1, y, z, blockIdData);
					break;
				}

				auto blockModelDown = GetInternalBlockModel(blockIdDown, idModels);
				auto blockModelUp = GetInternalBlockModel(blockIdUp, idModels);
				auto blockModelNorth = GetInternalBlockModel(blockIdNorth, idModels);
				auto blockModelSouth = GetInternalBlockModel(blockIdSouth, idModels);
				auto blockModelWest = GetInternalBlockModel(blockIdWest, idModels);
				auto blockModelEast = GetInternalBlockModel(blockIdEast, idModels);

				value |= (blockIdDown.id != 0 && blockModelDown && blockModelDown->IsBlock) << 0;
				value |= (blockIdUp.id != 0 && blockModelUp && blockModelUp->IsBlock) << 1;
				value |= (blockIdNorth.id != 0 && blockModelNorth && blockModelNorth->IsBlock) << 2;
				value |= (blockIdSouth.id != 0 && blockModelSouth && blockModelSouth->IsBlock) << 3;
				value |= (blockIdWest.id != 0 && blockModelWest && blockModelWest->IsBlock) << 4;
				value |= (blockIdEast.id != 0 && blockModelEast && blockModelEast->IsBlock) << 5;

				arr[y * 256 + z * 16 + x] = value;
			}
		}
	}
	return arr;
}

std::array<BlockId, 4096> SetBlockIdData(const SectionsData &sections) {
	std::array<BlockId, 4096> blockIdData;
	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				blockIdData[y * 256 + z * 16 + x] = sections.section.GetBlockId(Vector(x, y, z));
			}
		}
	}
	return blockIdData;
}

RendererSectionData ParseSection(const SectionsData &sections)
{
	RendererSectionData data;

	std::vector<std::pair<BlockId, const BlockModel *>> idModels;
	std::array<BlockId, 4096> blockIdData = SetBlockIdData(sections);
	std::array<unsigned char, 4096> blockVisibility = GetBlockVisibilityData(sections, blockIdData, idModels);
	std::string textureName;
	
	const std::map<BlockTextureId, glm::vec4> &textureAtlas = AssetManager::Instance().GetTextureAtlasIndexes();
	data.hash = sections.section.GetHash();
	data.sectionPos = sections.section.GetPosition();

	glm::mat4 baseOffset = glm::translate(glm::mat4(), (sections.section.GetPosition() * 16).glm()), transform;

	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				BlockId block = GetBlockId(x, y, z, blockIdData);
				if (block.id == 0)
					continue;

				Vector vec(x, y, z);

				transform = glm::translate(baseOffset, vec.glm());

				unsigned char light = sections.GetLight(vec);
				unsigned char skyLight = sections.GetSkyLight(vec);

				const BlockModel* model = GetInternalBlockModel(block, idModels);
				if (model) {
					AddFacesByBlockModel(vec, *model, transform, light, skyLight, blockVisibility, textureName, data);
				}
				else {
					transform = glm::translate(transform, glm::vec3(0, 1, 0));

					if (block.id == 8 || block.id == 9) {
						data.textures.push_back(AssetManager::Instance().GetTextureByAssetName("minecraft/textures/blocks/water_still"));
						data.textures.back().w /= 32.0f;
						transform = glm::translate(transform, glm::vec3(0, -0.2, 0));
					}
					else
						data.textures.push_back(AssetManager::Instance().GetTextureByAssetName("minecraft/textures/blocks/tnt_side"));

					data.models.push_back(transform);
					data.colors.push_back(glm::vec3(0, 0, 0));
					data.lights.push_back(glm::vec2(light, skyLight));
				}

			}
		}
	}
	data.textures.shrink_to_fit();
	data.models.shrink_to_fit();
	data.colors.shrink_to_fit();

	return data;
}

unsigned char SectionsData::GetLight(const Vector & pos) const {
	const Vector directions[] = {
		Vector(0,0,0),
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char value = 0;

	for (const Vector &dir : directions) {
		Vector vec = pos + dir;
		unsigned char dirValue = 0;

		if (vec.x < 0 || vec.x > 15 || vec.y < 0 || vec.y > 15 || vec.z < 0 || vec.z > 15) {
			if (vec.x < 0)
				dirValue = east.GetBlockLight(Vector(15, vec.y, vec.z));
			if (vec.x > 15)
				dirValue = west.GetBlockLight(Vector(0, vec.y, vec.z));
			if (vec.y < 0)
				dirValue = bottom.GetBlockLight(Vector(vec.x, 15, vec.z));
			if (vec.y > 15)
				dirValue = top.GetBlockLight(Vector(vec.x, 0, vec.z));
			if (vec.z < 0)
				dirValue = south.GetBlockLight(Vector(vec.x, vec.y, 15));
			if (vec.z > 15)
				dirValue = north.GetBlockLight(Vector(vec.x, vec.y, 0));
		}
		else
			dirValue = section.GetBlockLight(vec);

		value = _max(value, dirValue);
	}
	return value;
}

unsigned char SectionsData::GetSkyLight(const Vector & pos) const {
	const Vector directions[] = {
		Vector(0,0,0),
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char value = 0;

	for (const Vector &dir : directions) {
		Vector vec = pos + dir;
		unsigned char dirValue = 0;

		if (vec.x < 0 || vec.x > 15 || vec.y < 0 || vec.y > 15 || vec.z < 0 || vec.z > 15) {
			if (vec.x < 0)
				dirValue = east.GetBlockSkyLight(Vector(15, vec.y, vec.z));
			if (vec.x > 15)
				dirValue = west.GetBlockSkyLight(Vector(0, vec.y, vec.z));
			if (vec.y < 0)
				dirValue = bottom.GetBlockSkyLight(Vector(vec.x, 15, vec.z));
			if (vec.y > 15)
				dirValue = top.GetBlockSkyLight(Vector(vec.x, 0, vec.z));
			if (vec.z < 0)
				dirValue = south.GetBlockSkyLight(Vector(vec.x, vec.y, 15));
			if (vec.z > 15)
				dirValue = north.GetBlockSkyLight(Vector(vec.x, vec.y, 0));
		}
		else
			dirValue = section.GetBlockSkyLight(vec);

		value = _max(value, dirValue);
	}
	return value;
}
