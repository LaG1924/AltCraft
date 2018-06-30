#pragma once

#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "Block.hpp"

class Texture;

struct TextureCoordinates {
	TextureCoordinates(float x = -1, float y = -1, float w = -1, float h = -1)
            : x(x), y(y), w(w), h(h) {}

	bool operator==(const TextureCoordinates &rhs) const {
		return x == rhs.x &&
		       y == rhs.y &&
		       w == rhs.w &&
		       h == rhs.h;
	}

	explicit operator bool() const {
		return !(*this == TextureCoordinates(-1, -1, -1, -1));
	}

	double x, y, w, h;

    operator glm::vec4() const {
        return glm::vec4(x, y, w, h);
    }
};

struct BlockTextureId {
	//Block sides: 0 - bottom, 1 - top, 2 - north, 3 - south, 4 - west, 5 - east 6 - every side
	BlockTextureId(int id = 0, int state = 0, int side = 6)
            : id(id), state(state), side(side) {}

	int id : 9;
	int state : 4;
	int side : 3;


	bool operator<(const BlockTextureId &rhs) const {
		if (id < rhs.id)
			return true;
		if (rhs.id < id)
			return false;
		if (state < rhs.state)
			return true;
		if (rhs.state < state)
			return false;
		return side < rhs.side;
	}
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

        enum FaceDirection {
            down,
            up,
            north,
            south,
            west,
            east,
            none,
        };

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

	struct ParsedFace {
		ElementData::FaceDirection visibility;
		glm::mat4 transform;
		glm::vec4 texture;
		glm::vec3 color;
	};
	
	std::vector<ParsedFace> parsedFaces;
};

inline bool operator==(const BlockModel::ElementData::FaceData::Uv &lhs,
                       const BlockModel::ElementData::FaceData::Uv &rhs) {
    return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;
}

struct Asset {
	virtual ~Asset();
};

struct AssetTreeNode {
	std::vector<std::unique_ptr<AssetTreeNode>> childs;
	std::string name;
	AssetTreeNode *parent;
	std::unique_ptr<Asset> asset;
};

class AssetManager {
	Texture *textureAtlas;
	std::map<std::string, BlockId> assetIds;
	std::map<std::string, TextureCoordinates> assetTextures;
	std::map<BlockTextureId,glm::vec4> textureAtlasIndexes;
    std::map<std::string, BlockModel> models;
    std::map<BlockId, std::string> blockIdToBlockName;
	std::unique_ptr<AssetTreeNode> assetTree;
public:
	AssetManager();

	~AssetManager();

	void LoadTextureResources();

	TextureCoordinates GetTextureByAssetName(const std::string &AssetName);

	std::string GetTextureAssetNameByBlockId(BlockTextureId block);

	GLuint GetTextureAtlas();

	const std::map<BlockTextureId,glm::vec4> &GetTextureAtlasIndexes();

	void LoadIds();

	TextureCoordinates GetTextureByBlock(BlockTextureId block);

	static AssetManager& Instance();

    const BlockModel *GetBlockModelByBlockId(BlockId block);

    void LoadBlockModels();

    std::string GetAssetNameByBlockId(BlockId block);

	void ParseBlockModels();

	void LoadAssets();

	template <typename T>
	T *GetAsset(const std::string &assetName) {
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
		return dynamic_cast<T*>(node->asset.get());
	}
};
