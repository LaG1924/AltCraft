#include "RendererSectionData.hpp"

#include <array>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <optick.h>

#include "World.hpp"

inline const BlockId& GetBlockId(const Vector& pos, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[pos.y * 256 + pos.z * 16 + pos.x];
}

inline const BlockId& GetBlockId(int x, int y, int z, const std::array<BlockId, 4096> &blockIdData) {
	return blockIdData[y * 256 + z * 16 + x];
}

glm::vec2 TransformTextureCoord(glm::vec4 TextureAtlasCoords, glm::vec2 UvCoords, float frames) {
	float x = TextureAtlasCoords.x;
	float y = TextureAtlasCoords.y;
	float w = TextureAtlasCoords.z;
	float h = TextureAtlasCoords.w / frames;
	glm::vec2 transformed = glm::vec2(x, 1 - y - h) + UvCoords * glm::vec2(w, h);
	return transformed;
}

//Maps [0.0, 0.5] range to [0.0, 1.0]
float MapNeg(float x) {
    float y = x * 2.0f;
    return glm::clamp(y, 0.0f, 1.0f);
}

//Maps [0.5, 1.0] range to [0.0, 1.0]
float MapPos(float x) {
    float y = (x * 2.0f) - 1.0f;
    return glm::clamp(y, 0.0f, 1.0f);
}

float InterpolateBlockLightness(const BlockLightness& light, const glm::vec3 &point) {
    float xNeg = MapNeg(point.x);
    float xPos = MapPos(point.x);
    float xNegLight = glm::mix(light.face[FaceDirection::east], light.self, xNeg);
    float xPosLight = glm::mix(light.self, light.face[FaceDirection::west], xPos);
    float xLight = (glm::max)(xNegLight, xPosLight);

    float yNeg = MapNeg(point.y);
    float yPos = MapPos(point.y);
    float yNegLight = glm::mix(light.face[FaceDirection::down], light.self, yNeg);
    float yPosLight = glm::mix(light.self, light.face[FaceDirection::up], yPos);
    float yLight = (glm::max)(yNegLight, yPosLight);

    float zNeg = MapNeg(point.z);
    float zPos = MapPos(point.z);
    float zNegLight = glm::mix(light.face[FaceDirection::south], light.self, zNeg);
    float zPosLight = glm::mix(light.self, light.face[FaceDirection::north], zPos);
    float zLight = (glm::max)(zNegLight, zPosLight);

    return (glm::max)(xLight, (glm::max)(yLight, zLight));
}

void AddFacesByBlockModel(RendererSectionData& data, const BlockFaces& model, const glm::mat4& transform, bool visibility[FaceDirection::none], const Vector &pos, const SectionsData &sections) {
    glm::vec3 absPos = (sections.data[1][1][1].GetPosition() * 16).glm();
    for (const auto& face : model.faces) {
        if (face.visibility != FaceDirection::none) {
			FaceDirection direction = face.visibility;
			Vector directionVec = model.faceDirectionVector[direction];
			FaceDirection faceDirection = FaceDirection::none;
			for (int i = 0; i < FaceDirection::none; i++) {
				if (FaceDirectionVector[i] == directionVec) {
					faceDirection = FaceDirection(i);
					break;
				}
			}
			if (faceDirection == FaceDirection::none)
				continue;

			if (visibility[faceDirection])
				continue;
		}

		data.vertices.emplace_back();
		VertexData& vertexData = data.vertices.back();

		glm::mat4 transformed = transform * model.transform * face.transform;
		vertexData.positions[0] = transformed * glm::vec4(0, 0, 0, 1);
		vertexData.positions[1] = transformed * glm::vec4(0, 0, 1, 1);
		vertexData.positions[2] = transformed * glm::vec4(1, 0, 1, 1);
		vertexData.positions[3] = transformed * glm::vec4(1, 0, 0, 1);

        glm::vec3 normal = glm::cross(vertexData.positions[1] - vertexData.positions[0], vertexData.positions[3] - vertexData.positions[0]);
        vertexData.normal = glm::normalize(normal);

		vertexData.uvs[0] = TransformTextureCoord(face.texture, glm::vec2(0, 0), face.frames);
		vertexData.uvs[1] = TransformTextureCoord(face.texture, glm::vec2(1, 0), face.frames);
		vertexData.uvs[2] = TransformTextureCoord(face.texture, glm::vec2(1, 1), face.frames);
		vertexData.uvs[3] = TransformTextureCoord(face.texture, glm::vec2(0, 1), face.frames);

		vertexData.layerAnimationAo.r = face.layer;
		vertexData.layerAnimationAo.g = face.frames;
		vertexData.colors = face.color;

        bool useSmoothLighting = true;
        if (useSmoothLighting) {
            for (size_t i = 0; i < 4; i++) {
                glm::vec3 baseLightPos = vertexData.positions[i] - absPos;
                glm::vec3 lightPos = baseLightPos + normal * 0.5f;
                glm::ivec3 basePos = glm::trunc(lightPos);
                BlockLightness light = sections.GetLight(Vector(basePos.x, basePos.y, basePos.z));
				BlockLightness skyLight = sections.GetSkyLight(Vector(basePos.x, basePos.y, basePos.z));
                vertexData.lights[i].x = InterpolateBlockLightness(light, lightPos - glm::vec3(basePos));
                vertexData.lights[i].y = InterpolateBlockLightness(skyLight, lightPos - glm::vec3(basePos));
            }
        } else {
            BlockLightness light = sections.GetLight(pos);
            BlockLightness skyLight = sections.GetSkyLight(pos);
            glm::vec2 lightness;

            lightness.x = face.visibility != FaceDirection::none ? light.face[face.visibility] : light.self;
            lightness.x = (glm::max)(lightness.x, static_cast<float>(light.self));

            lightness.y = face.visibility != FaceDirection::none ? skyLight.face[face.visibility] : skyLight.self;
            lightness.y = (glm::max)(lightness.y, static_cast<float>(skyLight.self));

            vertexData.lights[0] = lightness;
            vertexData.lights[1] = lightness;
            vertexData.lights[2] = lightness;
            vertexData.lights[3] = lightness;
        }

        vertexData.layerAnimationAo.b = model.ambientOcclusion ? 1.0f : 0.0f;
    }
}

BlockFaces *GetInternalBlockModel(const BlockId& id, std::vector<std::pair<BlockId, BlockFaces*>> &idModels) {
    for (const auto& it : idModels) {
        if (it.first == id)
            return it.second;
    }
    idModels.push_back(std::make_pair(id, &AssetManager::GetBlockModelByBlockId(id)));
    return idModels.back().second;
}

std::array<bool[FaceDirection::none], 4096> GetBlockVisibilityData(const SectionsData &sections, const std::array<BlockId, 4096> &blockIdData, std::vector<std::pair<BlockId, BlockFaces*>> &idModels) {
	std::array<bool[FaceDirection::none], 4096> arr;
	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				unsigned char value = 0;

				Vector vec(x, y, z);

				BlockId blockIdDown = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::down]);
				BlockId blockIdUp = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::up]);
				BlockId blockIdNorth = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::north]);
				BlockId blockIdSouth = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::south]);
				BlockId blockIdWest = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::west]);
				BlockId blockIdEast = sections.GetBlockId(vec + FaceDirectionVector[FaceDirection::east]);

				auto blockModelDown = GetInternalBlockModel(blockIdDown, idModels);
				auto blockModelUp = GetInternalBlockModel(blockIdUp, idModels);
				auto blockModelNorth = GetInternalBlockModel(blockIdNorth, idModels);
				auto blockModelSouth = GetInternalBlockModel(blockIdSouth, idModels);
				auto blockModelWest = GetInternalBlockModel(blockIdWest, idModels);
				auto blockModelEast = GetInternalBlockModel(blockIdEast, idModels);
				
				arr[y * 256 + z * 16 + x][FaceDirection::down] = blockIdDown.id != 0 && !blockModelDown->faces.empty() && blockModelDown->isBlock;
				arr[y * 256 + z * 16 + x][FaceDirection::up] = blockIdUp.id != 0 && !blockModelUp->faces.empty() && blockModelUp->isBlock;
				arr[y * 256 + z * 16 + x][FaceDirection::north] = blockIdNorth.id != 0 && !blockModelNorth->faces.empty() && blockModelNorth->isBlock;
				arr[y * 256 + z * 16 + x][FaceDirection::south] = blockIdSouth.id != 0 && !blockModelSouth->faces.empty() && blockModelSouth->isBlock;
				arr[y * 256 + z * 16 + x][FaceDirection::west] = blockIdWest.id != 0 && !blockModelWest->faces.empty() && blockModelWest->isBlock;
				arr[y * 256 + z * 16 + x][FaceDirection::east] = blockIdEast.id != 0 && !blockModelEast->faces.empty() && blockModelEast->isBlock;
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
                blockIdData[y * 256 + z * 16 + x] = sections.data[1][1][1].GetBlockId(Vector(x, y, z));
			}
		}
	}
	return blockIdData;
}

RendererSectionData ParseSection(const SectionsData &sections) {
	OPTICK_EVENT();
	RendererSectionData data;

	std::vector<std::pair<BlockId, BlockFaces*>> idModels;
	std::array<BlockId, 4096> blockIdData = SetBlockIdData(sections);
	std::array<bool[FaceDirection::none], 4096> blockVisibility = GetBlockVisibilityData(sections, blockIdData, idModels);
	std::string textureName;

    data.hash = sections.data[1][1][1].GetHash();
    data.sectionPos = sections.data[1][1][1].GetPosition();

    glm::mat4 baseOffset = glm::translate(glm::mat4(1.0), (sections.data[1][1][1].GetPosition() * 16).glm()), transform;

	for (int y = 0; y < 16; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				BlockId block = GetBlockId(x, y, z, blockIdData);
				if (block.id == 0)
					continue;

				Vector vec(x, y, z);

				transform = glm::translate(baseOffset, vec.glm());

				BlockFaces *model = GetInternalBlockModel(block, idModels);
                AddFacesByBlockModel(data, *model, transform, blockVisibility[y * 256 + z * 16 + x], vec, sections);
			}
		}
	}
	data.vertices.shrink_to_fit();

	return data;
}

BlockId SectionsData::GetBlockId(const Vector &pos) const {
    Vector sectionPos = pos;
    return GetSection(sectionPos).GetBlockId(sectionPos);
}

BlockLightness SectionsData::GetLight(const Vector& pos) const {
    BlockLightness lightness;
    for (size_t i = 0; i <= FaceDirection::none; i++) {
        Vector vec = pos + FaceDirectionVector[i];
        uint8_t dirValue = GetSection(vec).GetBlockLight(vec);

        if (i == FaceDirection::none)
            lightness.self = dirValue;
        else
            lightness.face[i] = dirValue;
    }
    return lightness;
}

BlockLightness SectionsData::GetSkyLight(const Vector &pos) const {
    BlockLightness lightness;
    for (size_t i = 0; i <= FaceDirection::none; i++) {
        Vector vec = pos + FaceDirectionVector[i];
        uint8_t dirValue = GetSection(vec).GetBlockSkyLight(vec);

        if (i == FaceDirection::none)
            lightness.self = dirValue;
        else
            lightness.face[i] = dirValue;
    }
    return lightness;
}
