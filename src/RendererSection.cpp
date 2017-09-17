#include "RendererSection.hpp"

#include <thread>

RendererSection::RendererSection(RendererSectionData data) {
    glGenVertexArrays(1, &Vao);

    glGenBuffers(VBOCOUNT, Vbo);

    glBindVertexArray(Vao);
    {
        //Cube vertices
        GLuint VertAttribPos = 0;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[VERTICES]);
        glVertexAttribPointer(VertAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(VertAttribPos);

        //Cube UVs
        GLuint UvAttribPos = 2;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[UV]);
        glVertexAttribPointer(UvAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(UvAttribPos);

        //Textures
        GLuint textureAttribPos = 7;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
        glVertexAttribPointer(textureAttribPos, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(textureAttribPos);
        glCheckError();

        //Color
        GLuint colorAttribPos = 12;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
        glVertexAttribPointer(colorAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(colorAttribPos);

        //Light
        GLuint lightAttribPos = 13;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
        glVertexAttribPointer(lightAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(lightAttribPos);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
    glCheckError();


    //Upload data to VRAM
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(glm::vec3), data.vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[UV]);
    glBufferData(GL_ARRAY_BUFFER, data.uv.size() * sizeof(glm::vec2), data.uv.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
    glBufferData(GL_ARRAY_BUFFER, data.textures.size() * sizeof(glm::vec4), data.textures.data(), GL_DYNAMIC_DRAW);    

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
    glDrawArrays(GL_TRIANGLES, 0, numOfFaces);
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

    glm::mat4 baseOffset = glm::translate(glm::mat4(), (section.GetPosition() * 16).glm()), transform;

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
                    }
                    else
                        textures.push_back(AssetManager::Instance().GetTextureByAssetName("minecraft/textures/blocks/tnt_side"));

                    models.push_back(transform);
                    colors.push_back(glm::vec3(0, 0, 0));
                    lights.push_back(glm::vec2(16, 16));
                }

            }
        }
    }

    CreateVertices();
    models.clear();

    const int mul = 6;

    textures.resize(textures.size() * mul);
    for (int i = textures.size()/mul - 1; i > 0; i--) {
        textures[i * mul] = textures[i];
    }
    for (int i = 0; i < textures.size(); i += mul) {
        for (int j = 1; j < mul; j++) {
            textures[i + j] = textures[i];
        }
    }

    colors.resize(colors.size() * mul);
    for (int i = colors.size() / mul - 1; i > 0; i--) {
        colors[i * mul] = colors[i];
    }
    for (int i = 0; i < colors.size(); i += mul) {
        for (int j = 1; j < mul; j++) {
            colors[i + j] = colors[i];
        }
    }

    lights.resize(lights.size() * mul);
    for (int i = lights.size() / mul - 1; i > 0; i--) {
        lights[i * mul] = lights[i];
    }
    for (int i = 0; i < lights.size(); i += mul) {
        for (int j = 1; j < mul; j++) {
            lights[i + j] = lights[i];
        }
    }
    
    textures.shrink_to_fit();
    models.shrink_to_fit();
    colors.shrink_to_fit();
    lights.shrink_to_fit();
    vertices.shrink_to_fit();
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

void RendererSectionData::CreateVertices() {
    const GLfloat verts[] = {
        0, 0, 0,
        1, 0, 1,
        1, 0, 0,

        0, 0, 0,
        0, 0, 1,
        1, 0, 1,
    };

    const GLfloat uvs[] = {
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    for (const auto &model : models) {
        vertices.push_back(model * glm::vec4(verts[0], verts[1], verts[2], 1.0f));
        vertices.push_back(model * glm::vec4(verts[3], verts[4], verts[5], 1.0f));
        vertices.push_back(model * glm::vec4(verts[6], verts[7], verts[8], 1.0f));

        vertices.push_back(model * glm::vec4(verts[9], verts[10], verts[11], 1.0f));
        vertices.push_back(model * glm::vec4(verts[12], verts[13], verts[14], 1.0f));
        vertices.push_back(model * glm::vec4(verts[15], verts[16], verts[17], 1.0f));

        uv.push_back(glm::vec2(uvs[0], uvs[1]));
        uv.push_back(glm::vec2(uvs[2], uvs[3]));
        uv.push_back(glm::vec2(uvs[4], uvs[5]));

        uv.push_back(glm::vec2(uvs[6], uvs[7]));
        uv.push_back(glm::vec2(uvs[8], uvs[9]));
        uv.push_back(glm::vec2(uvs[10], uvs[11]));
    }
}