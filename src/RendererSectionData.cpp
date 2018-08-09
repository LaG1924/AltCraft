#include "RendererSectionData.hpp"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "World.hpp"

inline const BlockId& GetBlockId(const Vector& pos, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[pos.y * 256 + pos.z * 16 + pos.x];
}

inline const BlockId& GetBlockId(int x, int y, int z, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[y * 256 + z * 16 + x];
}

void AddFacesByBlockModel(RendererSectionData &data, const BlockModel &model, const glm::mat4 &transform, unsigned char visibility, BlockLightness light, BlockLightness skyLight) {
	for (const auto &face : model.parsedFaces) {
		glm::vec2 lightness;
		lightness.x = _max(light.face[0], light.face[1], light.face[2], light.face[3], light.face[4], light.face[5]);
		lightness.y = _max(skyLight.face[0], skyLight.face[1], skyLight.face[2], skyLight.face[3], skyLight.face[4], skyLight.face[5]);
		if (face.visibility != FaceDirection::none) {
			switch (face.visibility) {
				case FaceDirection::down: {
					if (visibility >> 0 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::down], skyLight.face[FaceDirection::down]);
					break;
				}
				case FaceDirection::up: {
					if (visibility >> 1 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::up], skyLight.face[FaceDirection::up]);
					break;
				}
				case FaceDirection::north: {
					if (visibility >> 2 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::north], skyLight.face[FaceDirection::north]);
					break;
				}
				case FaceDirection::south: {
					if (visibility >> 3 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::south], skyLight.face[FaceDirection::south]);
					break;
				}
				case FaceDirection::west: {
					if (visibility >> 4 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::west], skyLight.face[FaceDirection::west]);
					break;
				}
				case FaceDirection::east: {
					if (visibility >> 5 & 0x1)
						continue;
					lightness = glm::vec2(light.face[FaceDirection::east], skyLight.face[FaceDirection::east]);
					break;
				}
			}
		}

		data.models.push_back(transform * face.transform);
		data.textures.push_back(face.texture);
		data.textureLayers.push_back(face.layer);
		data.lights.push_back(lightness);
		data.colors.push_back(face.color);
	}
}

const BlockModel* GetInternalBlockModel(const BlockId& id, std::vector<std::pair<BlockId, const BlockModel *>> &idModels) {
    for (const auto& it : idModels) {
        if (it.first == id)
            return it.second;
    }
    idModels.push_back(std::make_pair(id, AssetManager::GetBlockModelByBlockId(id)));
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

				BlockLightness light = sections.GetLight(vec);
				BlockLightness skyLight = sections.GetSkyLight(vec);

				const BlockModel* model = GetInternalBlockModel(block, idModels);
				AddFacesByBlockModel(data, *model, transform, blockVisibility[y * 256 + z * 16 + x], light, skyLight);
			}
		}
	}
	data.textures.shrink_to_fit();
	data.textureLayers.shrink_to_fit();
	data.models.shrink_to_fit();
	data.colors.shrink_to_fit();

	return data;
}

BlockLightness SectionsData::GetLight(const Vector &pos) const {
	BlockLightness lightness;
	static const Vector directions[] = {
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char self = section.GetBlockLight(pos);

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

		dirValue = _max(self, dirValue);

		if (dir == directions[0])
			lightness.face[FaceDirection::west] = dirValue;
		if (dir == directions[1])
			lightness.face[FaceDirection::east] = dirValue;
		if (dir == directions[2])
			lightness.face[FaceDirection::up] = dirValue;
		if (dir == directions[3])
			lightness.face[FaceDirection::down] = dirValue;
		if (dir == directions[4])
			lightness.face[FaceDirection::north] = dirValue;
		if (dir == directions[5])
			lightness.face[FaceDirection::south] = dirValue;
	}
	return lightness;
}

BlockLightness SectionsData::GetSkyLight(const Vector &pos) const {
	BlockLightness lightness;
	static const Vector directions[] = {
		Vector(1,0,0),
		Vector(-1,0,0),
		Vector(0,1,0),
		Vector(0,-1,0),
		Vector(0,0,1),
		Vector(0,0,-1),
	};

	unsigned char self = section.GetBlockSkyLight(pos);

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

		dirValue = _max(self, dirValue);

		if (dir == directions[0])
			lightness.face[FaceDirection::west] = dirValue;
		if (dir == directions[1])
			lightness.face[FaceDirection::east] = dirValue;
		if (dir == directions[2])
			lightness.face[FaceDirection::up] = dirValue;
		if (dir == directions[3])
			lightness.face[FaceDirection::down] = dirValue;
		if (dir == directions[4])
			lightness.face[FaceDirection::north] = dirValue;
		if (dir == directions[5])
			lightness.face[FaceDirection::south] = dirValue;
	}
	return lightness;
}
