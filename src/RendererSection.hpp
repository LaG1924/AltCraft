#pragma once

#include <glm/mat4x4.hpp>
#include <GL/glew.h>

#include "Vector.hpp"

class RenderState;
class RendererSectionData;

class RendererSection {
    GLuint Vao = { 0 };
    GLuint Vbo = { 0 };
	
	size_t hash;
    Vector sectionPos;

    RendererSection(const RendererSection &other) = delete;
public:
    RendererSection(const RendererSectionData &data);

    RendererSection(RendererSection &&other);

	~RendererSection();

	void Render(RenderState &renderState);

    Vector GetPosition();

    size_t GetHash();

    size_t numOfFaces;

    friend void swap(RendererSection &lhs, RendererSection &rhs);

	void UpdateData(const RendererSectionData &data);
};