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

RendererSectionData::RendererSectionData(World * world, Vector sectionPosition) {
    const std::map<BlockTextureId, glm::vec4> &textureAtlas = AssetManager::Instance().GetTextureAtlasIndexes();
    const Section &section = world->GetSection(sectionPosition);

    auto sectionsList = world->GetSectionsList();

    for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                BlockId block = section.GetBlockId(Vector(x, y, z));
                if (block.id == 0)
                    continue;

                auto testBlockNonExist = [&](Vector block) -> bool {
                    Vector offset;
                    if (block.x == -1) {
                        offset = Vector(-1, 0, 0);
                        block.x = 15;
                    } else if (block.x == 16) {
                        offset = Vector(1, 0, 0);
                        block.x = 0;
                    } else if (block.y == -1) {
                        offset = Vector(0, -1, 0);
                        block.y = 15;
                    } else if (block.y == 16) {
                        offset = Vector(0, 1, 0);
                        block.y = 0;
                    } else if (block.z == -1) {
                        offset = Vector(0, 0, -1);
                        block.z = 15;
                    } else if (block.z == 16) {
                        offset = Vector(0, 0, 1);
                        block.z = 0;
                    }
                    if (offset != Vector(0, 0, 0)) {
                        if (std::find(sectionsList.begin(), sectionsList.end(), sectionPosition + offset) == sectionsList.end())
                            return true;
                        const Section& blockSection = world->GetSection(sectionPosition + offset);
                        return blockSection.GetBlockId(block).id == 0 || blockSection.GetBlockId(block).id == 31 || blockSection.GetBlockId(block).id == 18;
                    }                    
                    return  section.GetBlockId(block).id == 0 || section.GetBlockId(block).id == 31 || section.GetBlockId(block).id == 18;
                };

                unsigned char isVisible = 0;
                isVisible |= testBlockNonExist(Vector(x - 1, y, z)) << 0;
                isVisible |= testBlockNonExist(Vector(x + 1, y, z)) << 1;
                isVisible |= testBlockNonExist(Vector(x, y + 1, z)) << 2;
                isVisible |= testBlockNonExist(Vector(x, y - 1, z)) << 3;
                isVisible |= testBlockNonExist(Vector(x, y, z - 1)) << 4;
                isVisible |= testBlockNonExist(Vector(x, y, z + 1)) << 5;

                if (isVisible == 0x00)
                    continue;

                glm::mat4 transform;
                transform = glm::translate(transform, glm::vec3(sectionPosition * 16u));
                transform = glm::translate(transform, glm::vec3(x, y, z));
                glm::vec3 biomeColor(0.275, 0.63, 0.1);
                glm::vec3 color(0.0f, 0.0f, 0.0f);
                if (block.id == 31 || block.id == 18)
                    color = biomeColor;

                if (block.id == 31) { //X-cross like blocks rendering
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 2));
                    for (int i = 0; i < 4; i++) {
                        textures.push_back(texture->second);
                        colors.push_back(color);
                        lights.push_back(glm::vec2(0, 0));
                    }
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0.15f, 0, 0.15f));
                    faceTransform = glm::scale(faceTransform, glm::vec3(1.0f, 0.9f, 1.0f));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
                    faceTransform = glm::rotate(faceTransform, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0));
                    for (int i = 0; i < 4; i++) {
                        models.push_back(faceTransform);
                        faceTransform = glm::translate(faceTransform, glm::vec3(0.0f, 0.0f, 0.5f));
                        faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        faceTransform = glm::translate(faceTransform, glm::vec3(0.0f, 0.0f, -0.5f));
                    }
                    continue;
                }

                if (isVisible >> 0 & 0x1) { //east side of block (X+)
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 0));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 2));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    colors.push_back(color);
                    lights.push_back(glm::vec2(0, 0));
                }
                if (isVisible >> 1 & 0x1) { //west side X-
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(1, 0, 0));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0, 0.0f, 1.0f));
                    faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 3));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    colors.push_back(color);
                    lights.push_back(glm::vec2(0, 0));
                }
                if (isVisible >> 2 & 0x1) { //Top side Y+
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 1, 0));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 1));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    if (block.id != 2)
                        colors.push_back(color);
                    else
                        colors.push_back(biomeColor);
                    lights.push_back(glm::vec2(0, 0));
                }
                if (isVisible >> 3 & 0x1) { //Bottom side Y-
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 0));
                    faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1.0f, 0, 0));
                    faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 0));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    colors.push_back(color);
                    lights.push_back(glm::vec2(0, 0));
                }
                if (isVisible >> 4 & 0x1) { //south side Z+
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(1, 0, 0));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 3));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    colors.push_back(color);
                    lights.push_back(glm::vec2(0, 0));
                }
                if (isVisible >> 5 & 0x1) { //north side Z-
                    glm::mat4 faceTransform = glm::translate(transform, glm::vec3(0, 0, 1));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
                    faceTransform = glm::rotate(faceTransform, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                    faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1));
                    faceTransform = glm::rotate(faceTransform, glm::radians(180.0f), glm::vec3(1, 0, 0.0f));
                    faceTransform = glm::translate(faceTransform, glm::vec3(0, 0, -1.0f));
                    models.push_back(faceTransform);
                    auto texture = textureAtlas.find(BlockTextureId(block.id, block.state, 4));
                    if (texture != textureAtlas.end())
                        textures.push_back(texture->second);
                    else
                        textures.push_back(glm::vec4(0.0546875, 0.00442477876106194690,
                            0.0078125, 0.00442477876106194690)); //Fallback TNT texture
                    colors.push_back(color);
                    lights.push_back(glm::vec2(0, 0));
                }
            }
        }
    }
    hash = section.GetHash();
    sectionPos = sectionPosition;
    textures.shrink_to_fit();
    models.shrink_to_fit();
    colors.shrink_to_fit();
}

void swap(RendererSection & lhs, RendererSection & rhs) {
    std::swap(lhs.Vbo, rhs.Vbo);
    std::swap(lhs.Vao, rhs.Vao);
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.numOfFaces, rhs.numOfFaces);
    std::swap(lhs.sectionPos, rhs.sectionPos);
}
