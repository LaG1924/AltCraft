#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

#include <GL/glew.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <sol_forward.hpp>

#include "Vector.hpp"
#include "Block.hpp"
#include "TextureAtlas.hpp"
#include "Shader.hpp"

enum FaceDirection {
	down,
	up,
	north,
	south,
	west,
	east,
	none,
};

static const Vector FaceDirectionVector[] = {
	Vector(0,-1,0),
	Vector(0,1,0),
	Vector(0,0,-1),
	Vector(0,0,1),
	Vector(-1,0,0),
	Vector(1,0,0),
	Vector(0,0,0)
};

struct ParsedFace {
	FaceDirection visibility;
	glm::mat4 transform;
	glm::vec4 texture;
	float layer;
	float frames;
	glm::vec3 color;
};

struct BlockFaces {
	glm::mat4 transform;
	std::vector<ParsedFace> faces;
	bool isBlock;
	Vector faceDirectionVector[FaceDirection::none];
};

struct BlockModel {
    bool IsBlock = false;
    std::string BlockName;

    bool AmbientOcclusion = true;

    enum DisplayVariants {
        thirdperson_righthand,
        thirdperson_lefthand,
        firstperson_righthand,
        firstperson_lefthand,
        gui,
        head,
        ground,
        fixed,
        DisplayVariantsCount,
    };

    struct DisplayData {
        Vector rotation;
        Vector translation;
        Vector scale;
    };
    std::map<DisplayVariants, DisplayData> Display;

    std::map<std::string, std::string> Textures;

    struct ElementData {
        Vector from;
        Vector to;

        Vector rotationOrigin = Vector(8, 8, 8);
        enum Axis {
            x,
            y,
            z,
        } rotationAxis = Axis::x;
        int rotationAngle = 0;
        bool rotationRescale = false;

        bool shade = true;

        struct FaceData {
            struct Uv {
                int x1, y1, x2, y2;                
            } uv = { 0,0,0,0 };

            std::string texture;
            FaceDirection cullface = FaceDirection::none;
            int rotation = 0;
            bool tintIndex = false;
            
        };
        std::map<FaceDirection, FaceData> faces;
    };

    std::vector<ElementData> Elements;

	std::vector<ParsedFace> parsedFaces;
};

struct BlockStateVariant {
	std::string variantName;

	struct Model {
		std::string modelName;
		int x = 0;
		int y = 0;
		bool uvLock = false;
		int weight = 1;
	};

	std::vector<Model> models;
};

struct BlockState {
	std::map<std::string, BlockStateVariant> variants;
};

inline bool operator==(const BlockModel::ElementData::FaceData::Uv &lhs,
                       const BlockModel::ElementData::FaceData::Uv &rhs) {
    return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;
}

struct Asset {
	virtual ~Asset() {};
};

struct AssetTreeNode {
	std::vector<std::unique_ptr<AssetTreeNode>> childs;
	std::vector<unsigned char> data;
	std::string name;
	std::unique_ptr<Asset> asset;
	AssetTreeNode *parent;
};

struct AssetBlockModel : Asset {
	BlockModel blockModel;
};

struct AssetBlockState : Asset {
	BlockState blockState;
};

struct AssetTexture : Asset {
	std::vector<unsigned char> textureData;
	unsigned int realWidth, realHeight;
	unsigned int frames;
	size_t id;
};

struct AssetShader : Asset {
	std::unique_ptr<Shader> shader;
};

struct AssetScript : Asset {
	std::string code;
};

namespace AssetManager {
	void InitAssetManager();

	BlockFaces &GetBlockModelByBlockId(BlockId block);

    std::string GetAssetNameByBlockId(BlockId block);

	Asset *GetAssetPtr(const std::string &assetName);

	template <typename T>
	T *GetAsset(const std::string &assetName) {		
		return dynamic_cast<T*>(GetAssetPtr(assetName));
	}

	void RecursiveWalkAsset(const std::string &assetPath, std::function<void(AssetTreeNode&)> fnc);

	AssetTreeNode *GetAssetByAssetName(const std::string &assetName);
	
	GLuint GetTextureAtlasId();

	TextureCoord GetTexture(const std::string assetName);
};
