#include "RendererSection.hpp"

#include <thread>

const GLfloat vertices[] = {
		0, 0, 0,
		1, 0, 1,
		1, 0, 0,

		0, 0, 0,
		0, 0, 1,
		1, 0, 1,
};

const GLfloat uv_coords[] = {
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
};

const GLuint magicUniqueConstant = 88375;
GLuint RendererSection::VboVertices = magicUniqueConstant;
GLuint RendererSection::VboUvs;
GLuint RendererSection::Vao;
GLuint RendererSection::Vbo[VBOCOUNT];

std::vector<glm::mat4> contentModels;
std::vector<glm::vec4> contentTextures;
std::vector<glm::vec3> contentColors;
std::vector<glm::vec2> contentLights;

RendererSection::RendererSection(RendererSectionData data) {
    if (VboVertices == magicUniqueConstant) {
        glGenBuffers(1, &VboVertices);
        glGenBuffers(1, &VboUvs);

        //Cube vertices
        glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        //Cube UVs
        glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), uv_coords, GL_STATIC_DRAW);

        LOG(INFO) << "Created VBOs with vertices (" << VboVertices << ") and UVs (" << VboUvs
            << ") for faces";

        glGenVertexArrays(1, &Vao);
        glGenBuffers(VBOCOUNT, Vbo);

        glBindVertexArray(Vao);
        {
            //Cube vertices
            GLuint VertAttribPos = 0;
            glBindBuffer(GL_ARRAY_BUFFER, VboVertices);
            glVertexAttribPointer(VertAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(VertAttribPos);

            //Cube UVs
            GLuint UvAttribPos = 2;
            glBindBuffer(GL_ARRAY_BUFFER, VboUvs);
            glVertexAttribPointer(UvAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(UvAttribPos);

            //Textures
            GLuint textureAttribPos = 7;
            glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
            glVertexAttribPointer(textureAttribPos, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(textureAttribPos);
            glVertexAttribDivisor(textureAttribPos, 1);
            glCheckError();

            //Blocks models
            GLuint matAttribPos = 8;
            size_t sizeOfMat4 = 4 * 4 * sizeof(GLfloat);
            glBindBuffer(GL_ARRAY_BUFFER, Vbo[MODELS]);
            glVertexAttribPointer(matAttribPos + 0, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, nullptr);
            glVertexAttribPointer(matAttribPos + 1, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *)(1 * 4 * sizeof(GLfloat)));
            glVertexAttribPointer(matAttribPos + 2, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *)(2 * 4 * sizeof(GLfloat)));
            glVertexAttribPointer(matAttribPos + 3, 4, GL_FLOAT, GL_FALSE, sizeOfMat4, (void *)(3 * 4 * sizeof(GLfloat)));
            glEnableVertexAttribArray(matAttribPos + 0);
            glEnableVertexAttribArray(matAttribPos + 1);
            glEnableVertexAttribArray(matAttribPos + 2);
            glEnableVertexAttribArray(matAttribPos + 3);
            glVertexAttribDivisor(matAttribPos + 0, 1);
            glVertexAttribDivisor(matAttribPos + 1, 1);
            glVertexAttribDivisor(matAttribPos + 2, 1);
            glVertexAttribDivisor(matAttribPos + 3, 1);

            //Color
            GLuint colorAttribPos = 12;
            glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
            glVertexAttribPointer(colorAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(colorAttribPos);
            glVertexAttribDivisor(colorAttribPos, 1);

            //Light
            GLuint lightAttribPos = 13;
            glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
            glVertexAttribPointer(lightAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(lightAttribPos);
            glVertexAttribDivisor(lightAttribPos, 1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
        glCheckError();
    }

    //Upload data to VRAM
    contentModels.insert(contentModels.end(), data.models.begin(), data.models.end());
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[MODELS]);
    glBufferData(GL_ARRAY_BUFFER, contentModels.size() * sizeof(glm::mat4), contentModels.data(), GL_DYNAMIC_DRAW);
    
    contentTextures.insert(contentTextures.end(), data.textures.begin(), data.textures.end());
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
    glBufferData(GL_ARRAY_BUFFER, contentTextures.size() * sizeof(glm::vec4), contentTextures.data(), GL_DYNAMIC_DRAW);

    contentColors.insert(contentColors.end(), data.colors.begin(), data.colors.end());
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
    glBufferData(GL_ARRAY_BUFFER, contentColors.size() * sizeof(glm::vec3), contentColors.data(), GL_DYNAMIC_DRAW);

    contentLights.insert(contentLights.end(), data.lights.begin(), data.lights.end());
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
    glBufferData(GL_ARRAY_BUFFER, contentLights.size() * sizeof(glm::vec2), contentLights.data(), GL_DYNAMIC_DRAW);    

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    numOfFaces = data.textures.size();
    offset = contentModels.size() - numOfFaces;
    sectionPos = data.sectionPos;
    hash = data.hash;
}

RendererSection::RendererSection(RendererSection && other) {
    using std::swap;
    swap(*this, other);
}

RendererSection::~RendererSection() {
}

void swap(RendererSection & lhs, RendererSection & rhs) {
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.numOfFaces, rhs.numOfFaces);
    std::swap(lhs.sectionPos, rhs.sectionPos);
    std::swap(lhs.offset, rhs.offset);
}

Vector RendererSection::GetPosition()
{
    return sectionPos;
}

size_t RendererSection::GetHash()
{
    return hash;
}

GLuint RendererSection::GetVao() {
    return Vao;
}

RendererSectionData::RendererSectionData(World * world, Vector sectionPosition) {    
    const std::map<BlockTextureId, glm::vec4> &textureAtlas = AssetManager::Instance().GetTextureAtlasIndexes();
    const Section &section = world->GetSection(sectionPosition);
    hash = section.GetHash();
    sectionPos = sectionPosition;

    glm::mat4 baseOffset = glm::translate(glm::mat4(), (section.GetPosition() * 16).glm()),transform;

    auto sectionsList = world->GetSectionsList();

    for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                BlockId block = section.GetBlockId(Vector(x, y, z));
                if (block.id == 0)
                    continue;

                const bool useNewMethod = true;


                transform = glm::translate(baseOffset, Vector(x, y, z).glm());

                const BlockModel* model = AssetManager::Instance().GetBlockModelByBlockId(block);
                if (model) {
                    this->AddFacesByBlockModel(sectionsList, world, Vector(x, y, z), *model, transform, section.GetBlockLight(Vector(x, y, z)), section.GetBlockSkyLight(Vector(x, y, z)));
                }
                else {
                    transform = glm::translate(transform, glm::vec3(0, 1, 0));
                    
                    if (block.id == 8 || block.id == 9) {
                        textures.push_back(AssetManager::Instance().GetTextureByAssetName("minecraft/textures/blocks/water_still"));
                        textures.back().w /= 32.0f;
                        transform = glm::translate(transform, glm::vec3(0, -0.2, 0));
                    } else
                        textures.push_back(AssetManager::Instance().GetTextureByAssetName("minecraft/textures/blocks/tnt_side"));

                    models.push_back(transform);
                    colors.push_back(glm::vec3(0, 0, 0));
                    lights.push_back(glm::vec2(16, 16));
                }

            }
        }
    }
    textures.shrink_to_fit();
    models.shrink_to_fit();
    colors.shrink_to_fit();
}

void RendererSectionData::AddFacesByBlockModel(const std::vector<Vector> &sectionsList, World *world, Vector blockPos, const BlockModel &model, glm::mat4 transform, unsigned char light, unsigned char skyLight) {
    glm::mat4 elementTransform, faceTransform;
    for (const auto& element : model.Elements) {
        Vector t = element.to - element.from;
        VectorF elementSize(VectorF(t.x,t.y,t.z) / 16.0f);
        VectorF elementOrigin(VectorF(element.from.x,element.from.y,element.from.z) / 16.0f);
        elementTransform = glm::translate(transform, elementOrigin.glm());
        elementTransform = glm::scale(elementTransform, elementSize.glm());

        for (const auto& face : element.faces) {
            if (face.second.cullface != BlockModel::ElementData::FaceDirection::none) {
                switch (face.second.cullface) {
                case BlockModel::ElementData::FaceDirection::down:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(0, +1, 0)))
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::up:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(0, -1, 0)))
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::north:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(0, 0, -1)))
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::south:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(0, 0, +1)))
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::west:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(-1, 0, 0)))
                        continue;
                    break;
                case BlockModel::ElementData::FaceDirection::east:
                    if (TestBlockExists(sectionsList, world, blockPos - Vector(+1, 0, 0)))
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
            models.push_back(faceTransform);
            std::string textureName = face.second.texture;
            while (textureName[0] == '#') {
                textureName = model.Textures.find(std::string(textureName.begin()+1,textureName.end()))->second;
            }
            glm::vec4 texture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/" + textureName);
            textures.push_back(texture);
            if (face.second.tintIndex)
                colors.push_back(glm::vec3(0.275, 0.63, 0.1));
            else
                colors.push_back(glm::vec3(0, 0, 0));
            lights.push_back(glm::vec2(light, skyLight));
        }
    }
}

bool RendererSectionData::TestBlockExists(const std::vector<Vector> &sectionsList, World *world, Vector blockPos) {
    Vector section = sectionPos;
    if (blockPos.x == -1) {
        section = section + Vector(-1, 0, 0);
        blockPos.x = 15;
    }
    else if (blockPos.x == 16) {
        section = section + Vector(+1, 0, 0);
        blockPos.x = 0;
    }
    else if (blockPos.y == -1) {
        section = section + Vector(0, -1, 0);
        blockPos.y = 15;
    }
    else if (blockPos.y == 16) {
        section = section + Vector(0, +1, 0);
        blockPos.y = 0;
    }
    else if (blockPos.z == -1) {
        section = section + Vector(0, 0, -1);
        blockPos.z = 15;
    }
    else if (blockPos.z == 16) {
        section = section + Vector(0, 0, +1);
        blockPos.z = 0;
    }
    
    if (std::find(sectionsList.begin(), sectionsList.end(),section) == sectionsList.end())
        return true;

    BlockId blockId = world->GetSection(section).GetBlockId(blockPos);

    auto blockModel = AssetManager::Instance().GetBlockModelByBlockId(world->GetSection(section).GetBlockId(blockPos));

    return blockId.id != 0 && blockModel && blockModel->IsBlock;
}