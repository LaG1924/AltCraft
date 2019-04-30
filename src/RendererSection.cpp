#include "RendererSection.hpp"

#include <easylogging++.h>
#include <optick.h>

#include "Utility.hpp"
#include "Renderer.hpp"
#include "RendererSectionData.hpp"

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

RendererSection::RendererSection(const RendererSectionData &data) {
	OPTICK_EVENT();
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

		//TextureLayers
		GLuint layerAttribPos = 14;
		glBindBuffer(GL_ARRAY_BUFFER, Vbo[LAYERS]);
		glVertexAttribPointer(layerAttribPos, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(layerAttribPos);
		glVertexAttribDivisor(layerAttribPos, 1);
		glCheckError();

		//TextureFrames
		GLuint framesAttribPos = 15;
		glBindBuffer(GL_ARRAY_BUFFER, Vbo[FRAMES]);
		glVertexAttribPointer(framesAttribPos, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), nullptr);
		glEnableVertexAttribArray(framesAttribPos);
		glVertexAttribDivisor(framesAttribPos, 1);
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
	
	UpdateData(data);
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
	OPTICK_EVENT();
	renderState.SetActiveVao(Vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numOfFaces);    
	glCheckError();
}

Vector RendererSection::GetPosition() {
    return sectionPos;
}

size_t RendererSection::GetHash() {
    return hash;
}

void RendererSection::UpdateData(const RendererSectionData & data) {
	OPTICK_EVENT();
	glBindBuffer(GL_ARRAY_BUFFER, Vbo[TEXTURES]);
	glBufferData(GL_ARRAY_BUFFER, data.textures.size() * sizeof(glm::vec4), data.textures.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[LAYERS]);
	glBufferData(GL_ARRAY_BUFFER, data.textureLayers.size() * 1* sizeof(GLfloat), data.textureLayers.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[FRAMES]);
	glBufferData(GL_ARRAY_BUFFER, data.textureFrames.size() * 1 * sizeof(GLfloat), data.textureFrames.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[MODELS]);
	glBufferData(GL_ARRAY_BUFFER, data.models.size() * sizeof(glm::mat4), data.models.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
	glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(glm::vec3), data.colors.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
	glBufferData(GL_ARRAY_BUFFER, data.lights.size() * sizeof(glm::vec2), data.lights.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCheckError();

	numOfFaces = data.textures.size();
	sectionPos = data.sectionPos;
	hash = data.hash;
}
