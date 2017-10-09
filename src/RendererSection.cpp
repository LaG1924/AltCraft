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
GLuint RendererSection::VboUvs = magicUniqueConstant;

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
    }

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


    //Upload data to VRAM
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
    glBufferData(GL_ARRAY_BUFFER, data.textures.size() * sizeof(glm::vec4), data.textures.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[MODELS]);
    glBufferData(GL_ARRAY_BUFFER, data.models.size() * sizeof(glm::mat4), data.models.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
    glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(glm::vec3), data.colors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
    glBufferData(GL_ARRAY_BUFFER, data.lights.size() * sizeof(glm::vec2), data.lights.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    numOfFaces = data.textures.size();
    sectionPos = data.sectionPos;
    hash = data.hash;
}

RendererSection::RendererSection(RendererSection && other) {
    using std::swap;
    swap(*this, other);
}

RendererSection::~RendererSection() {
    if (Vao != 0)
        glDeleteVertexArrays(1, &Vao);
    
    for (int i = 0; i < VBOCOUNT; i++)
        if (Vbo[i] != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, Vbo[i]);
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
        }

    glDeleteBuffers(VBOCOUNT, Vbo);
}

void swap(RendererSection & lhs, RendererSection & rhs) {
    std::swap(lhs.Vbo, rhs.Vbo);
    std::swap(lhs.Vao, rhs.Vao);
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.numOfFaces, rhs.numOfFaces);
    std::swap(lhs.sectionPos, rhs.sectionPos);
}

void RendererSection::Render(RenderState &renderState) {
	renderState.SetActiveVao(Vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numOfFaces);    
	glCheckError();
}

Vector RendererSection::GetPosition()
{
    return sectionPos;
}

size_t RendererSection::GetHash()
{
    return hash;
}

RendererSectionData::RendererSectionData(World * world, Vector sectionPosition) {    
    const std::map<BlockTextureId, glm::vec4> &textureAtlas = AssetManager::Instance().GetTextureAtlasIndexes();
    const Section &section = world->GetSection(sectionPosition);
    hash = section.GetHash();
    sectionPos = sectionPosition;

    SetBlockIdData(world);

    auto blockVisibility = GetBlockVisibilityData(world);

    glm::mat4 baseOffset = glm::translate(glm::mat4(), (section.GetPosition() * 16).glm()),transform;

    auto sectionsList = world->GetSectionsList();

    for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                BlockId block = GetBlockId(x,y,z);
                if (block.id == 0)
                    continue;

                const bool useNewMethod = true;


                transform = glm::translate(baseOffset, Vector(x, y, z).glm());

                const BlockModel* model = this->GetInternalBlockModel(block);
                if (model) {
                    this->AddFacesByBlockModel(sectionsList, world, Vector(x, y, z), *model, transform, section.GetBlockLight(Vector(x, y, z)), section.GetBlockSkyLight(Vector(x, y, z)), blockVisibility);
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

void RendererSectionData::AddFacesByBlockModel(const std::vector<Vector> &sectionsList, World *world, Vector blockPos, const BlockModel &model, glm::mat4 transform, unsigned char light, unsigned char skyLight, const std::array<unsigned char, 16 * 16 * 16>& visibility) {
    glm::mat4 elementTransform, faceTransform;
    for (const auto& element : model.Elements) {
        Vector t = element.to - element.from;
        VectorF elementSize(VectorF(t.x,t.y,t.z) / 16.0f);
        VectorF elementOrigin(VectorF(element.from.x,element.from.y,element.from.z) / 16.0f);
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
            models.push_back(faceTransform);
            std::string textureName = face.second.texture;
            while (textureName[0] == '#') {
                textureName = model.Textures.find(std::string(textureName.begin()+1,textureName.end()))->second;
            }
            glm::vec4 texture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/" + textureName);
            
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
            textures.push_back(texture);
            if (face.second.tintIndex)
                colors.push_back(glm::vec3(0.275, 0.63, 0.1));
            else
                colors.push_back(glm::vec3(0, 0, 0));
            lights.push_back(glm::vec2(light, skyLight));
        }
    }
}

std::array<unsigned char, 4096> RendererSectionData::GetBlockVisibilityData(World *world) {
    //const auto& section = world->GetSection(sectionPos);
    const auto& sectionDown = world->GetSection(sectionPos + Vector(0, -1, 0));
    const auto& sectionUp = world->GetSection(sectionPos + Vector(0, +1, 0));
    const auto& sectionNorth = world->GetSection(sectionPos + Vector(0, 0, +1));
    const auto& sectionSouth = world->GetSection(sectionPos + Vector(0, 0, -1));
    const auto& sectionWest = world->GetSection(sectionPos + Vector(+1, 0, 0));
    const auto& sectionEast = world->GetSection(sectionPos + Vector(-1, 0, 0));

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
                    blockIdDown = sectionDown.GetBlockId(Vector(x, 15, z));
                    blockIdUp = GetBlockId(x,1,z);  
                    break;
                case 15:
                    blockIdDown = GetBlockId(x,14,z);
                    blockIdUp = sectionUp.GetBlockId(Vector(x, 0, z));
                    break;
                default:
                    blockIdDown = GetBlockId(x, y -1, z);
                    blockIdUp = GetBlockId(x, y + 1, z);
                    break;
                }

                switch (z) {
                case 0:
                    blockIdNorth = GetBlockId(x, y, 1);
                    blockIdSouth = sectionSouth.GetBlockId(Vector(x, y, 15));
                    break;
                case 15:
                    blockIdNorth = sectionNorth.GetBlockId(Vector(x, y, 0));
                    blockIdSouth = GetBlockId(x, y, 14);         
                    break;
                default:
                    blockIdNorth = GetBlockId(x, y, z + 1);
                    blockIdSouth = GetBlockId(x, y, z - 1);
                    break;
                }

                switch (x) {
                case 0:
                    blockIdWest = GetBlockId(1, y, z);
                    blockIdEast = sectionEast.GetBlockId(Vector(15, y, z));
                    break;
                case 15:
                    blockIdWest = sectionWest.GetBlockId(Vector(0, y, z));
                    blockIdEast = GetBlockId(14, y, z);
                    break;
                default:
                    blockIdWest = GetBlockId(x + 1, y, z);
                    blockIdEast = GetBlockId(x - 1, y, z);
                    break;
                }

                auto blockModelDown = GetInternalBlockModel(blockIdDown);
                auto blockModelUp = GetInternalBlockModel(blockIdUp);
                auto blockModelNorth = GetInternalBlockModel(blockIdNorth);
                auto blockModelSouth = GetInternalBlockModel(blockIdSouth);
                auto blockModelWest = GetInternalBlockModel(blockIdWest);
                auto blockModelEast = GetInternalBlockModel(blockIdEast);

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

const BlockModel* RendererSectionData::GetInternalBlockModel(const BlockId& id) {
        for (const auto& it : idModels) {
            if (it.first == id)
                return it.second;
        }
        idModels.push_back(std::make_pair(id, am.GetBlockModelByBlockId(id)));
        return idModels.back().second;
}

void RendererSectionData::SetBlockIdData(World* world) {
    const Section& section = world->GetSection(sectionPos);

    for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                blockIdData[y * 256 + z * 16 + x] = section.GetBlockId(Vector(x,y,z));
            }
        }
    }
}