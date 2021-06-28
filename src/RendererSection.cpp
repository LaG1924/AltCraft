#include "RendererSection.hpp"

#include <cstddef>

#include <easylogging++.h>
#include <optick.h>

#include "Utility.hpp"
#include "Renderer.hpp"
#include "RendererSectionData.hpp"

RendererSection::RendererSection(const RendererSectionData &data) {
	OPTICK_EVENT();

    glGenVertexArrays(1, &Vao);
    
    glGenBuffers(1, &Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);

    glBindVertexArray(Vao);
    {
        //Cube vertices
        GLuint VertAttribPos = 0;
        glVertexAttribPointer(VertAttribPos, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, positions[0]));
        glEnableVertexAttribArray(VertAttribPos);
        glVertexAttribDivisor(VertAttribPos, 1);

        glVertexAttribPointer(VertAttribPos + 1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, positions[1]));
        glEnableVertexAttribArray(VertAttribPos + 1);
        glVertexAttribDivisor(VertAttribPos + 1, 1);

        glVertexAttribPointer(VertAttribPos + 2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, positions[2]));
        glEnableVertexAttribArray(VertAttribPos + 2);
        glVertexAttribDivisor(VertAttribPos + 2, 1);

        glVertexAttribPointer(VertAttribPos + 3, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, positions[3]));
        glEnableVertexAttribArray(VertAttribPos + 3);
        glVertexAttribDivisor(VertAttribPos + 3, 1);
        glCheckError();

        //Cube uvs
        GLuint UvAttribPos = 4;
        glVertexAttribPointer(UvAttribPos, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uvs[0]));
        glEnableVertexAttribArray(UvAttribPos);
        glVertexAttribDivisor(UvAttribPos, 1);

        glVertexAttribPointer(UvAttribPos + 1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uvs[1]));
        glEnableVertexAttribArray(UvAttribPos + 1);
        glVertexAttribDivisor(UvAttribPos + 1, 1);

        glVertexAttribPointer(UvAttribPos + 2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uvs[2]));
        glEnableVertexAttribArray(UvAttribPos + 2);
        glVertexAttribDivisor(UvAttribPos + 2, 1);

        glVertexAttribPointer(UvAttribPos + 3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uvs[3]));
        glEnableVertexAttribArray(UvAttribPos + 3);
        glVertexAttribDivisor(UvAttribPos + 3, 1);

        //Uv Layer
        GLuint uvLayerAttribPos = 8;
        glVertexAttribPointer(uvLayerAttribPos, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uvLayers));
        glEnableVertexAttribArray(uvLayerAttribPos);
        glVertexAttribDivisor(uvLayerAttribPos, 1);

        //Animation
        GLuint animationAttribPos = 9;
        glVertexAttribPointer(animationAttribPos, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, animations));
        glEnableVertexAttribArray(animationAttribPos);
        glVertexAttribDivisor(animationAttribPos, 1);

        //Color
        GLuint colorAttribPos = 10;
        glVertexAttribPointer(colorAttribPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, colors));
        glEnableVertexAttribArray(colorAttribPos);
        glVertexAttribDivisor(colorAttribPos, 1);

        size_t m = sizeof(VertexData);
        size_t d = offsetof(VertexData, lights);

        //Light
        GLuint lightAttribPos = 11;
        glVertexAttribPointer(lightAttribPos, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, lights));
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
    
    glDeleteBuffers(1, &Vbo);
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

    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(VertexData), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCheckError();

	numOfFaces = data.vertices.size();
	sectionPos = data.sectionPos;
	hash = data.hash;
}
