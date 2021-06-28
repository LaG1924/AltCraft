#include "RendererSection.hpp"

#include <easylogging++.h>
#include <optick.h>

#include "Utility.hpp"
#include "Renderer.hpp"
#include "RendererSectionData.hpp"

RendererSection::RendererSection(const RendererSectionData &data) {
	OPTICK_EVENT();

    glGenVertexArrays(1, &Vao);
    
    glGenBuffers(VBOCOUNT, Vbo);

    glBindVertexArray(Vao);
    {
        //Cube vertices
        GLuint VertAttribPos = 0;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[POSITIONS]);
        glVertexAttribPointer(VertAttribPos, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(VertAttribPos);
        glVertexAttribDivisor(VertAttribPos, 1);

        glVertexAttribPointer(VertAttribPos + 1, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat), (void*)(1 * 4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(VertAttribPos + 1);
        glVertexAttribDivisor(VertAttribPos + 1, 1);

        glVertexAttribPointer(VertAttribPos + 2, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat), (void*)(2 * 4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(VertAttribPos + 2);
        glVertexAttribDivisor(VertAttribPos + 2, 1);

        glVertexAttribPointer(VertAttribPos + 3, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat), (void*)(3 * 4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(VertAttribPos + 3);
        glVertexAttribDivisor(VertAttribPos + 3, 1);
        glCheckError();

        //Cube uvs
        GLuint UvAttribPos = 4;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[UVS]);
        glVertexAttribPointer(UvAttribPos, 2, GL_FLOAT, GL_FALSE, 4 * 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(UvAttribPos);
        glVertexAttribDivisor(UvAttribPos, 1);

        glVertexAttribPointer(UvAttribPos + 1, 2, GL_FLOAT, GL_FALSE, 4 * 2 * sizeof(GLfloat), (void*)(1 * 2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(UvAttribPos + 1);
        glVertexAttribDivisor(UvAttribPos + 1, 1);

        glVertexAttribPointer(UvAttribPos + 2, 2, GL_FLOAT, GL_FALSE, 4 * 2 * sizeof(GLfloat), (void*)(2 * 2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(UvAttribPos + 2);
        glVertexAttribDivisor(UvAttribPos + 2, 1);

        glVertexAttribPointer(UvAttribPos + 3, 2, GL_FLOAT, GL_FALSE, 4 * 2 * sizeof(GLfloat), (void*)(3 * 2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(UvAttribPos + 3);
        glVertexAttribDivisor(UvAttribPos + 3, 1);
        glCheckError();

        //Uv Layer
        GLuint uvLayerAttribPos = 8;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[UVLAYERS]);
        glVertexAttribPointer(uvLayerAttribPos, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(uvLayerAttribPos);
        glVertexAttribDivisor(uvLayerAttribPos, 1);
        glCheckError();

        //Animation
        GLuint animationAttribPos = 9;
        glCheckError();
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[ANIMATIONS]);
        glCheckError();
        glVertexAttribPointer(animationAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glCheckError();
        glEnableVertexAttribArray(animationAttribPos);
        glCheckError();
        glVertexAttribDivisor(animationAttribPos, 1);
        glCheckError();

        //Color
        GLuint colorAttribPos = 10;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
        glVertexAttribPointer(colorAttribPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(colorAttribPos);
        glVertexAttribDivisor(colorAttribPos, 1);
        glCheckError();

        //Light
        GLuint lightAttribPos = 11;
        glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
        glVertexAttribPointer(lightAttribPos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(lightAttribPos);
        glVertexAttribDivisor(lightAttribPos, 1);
        glCheckError();

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
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, numOfFaces);
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

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec4), data.positions.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[UVS]);
	glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(glm::vec2), data.uvs.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Vbo[UVLAYERS]);
	glBufferData(GL_ARRAY_BUFFER, data.uvLayers.size() * sizeof(GLfloat), data.uvLayers.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[ANIMATIONS]);
    glBufferData(GL_ARRAY_BUFFER, data.animations.size() * sizeof(glm::vec2), data.animations.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[COLORS]);
    glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(glm::vec3), data.colors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Vbo[LIGHTS]);
    glBufferData(GL_ARRAY_BUFFER, data.lights.size() * sizeof(glm::vec2), data.lights.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCheckError();

	numOfFaces = data.animations.size();
	sectionPos = data.sectionPos;
	hash = data.hash;
}
