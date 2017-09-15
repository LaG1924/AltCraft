#pragma once

#include <GL/glew.h>
#include <glm/detail/type_mat.hpp>
#include <glm/vec2.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <easylogging++.h>

#include "AssetManager.hpp"
#include "Section.hpp"
#include "World.hpp"
#include "Vector.hpp"
#include "Renderer.hpp"

struct RendererSectionData {
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> textures;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> lights;
    size_t hash;
    Vector sectionPos;

    RendererSectionData(World *world, Vector sectionPosition);
private:
    bool TestBlockExists(const std::vector<Vector> &sectionsList, World *world, Vector blockPos);

    void AddFacesByBlockModel(const std::vector<Vector> &sectionsList, World *world, Vector blockPos, const BlockModel &model, glm::mat4 transform, unsigned char light, unsigned char skyLight);
};
class RendererSection {
    enum Vbos {
        MODELS = 0,
        TEXTURES,
        COLORS,
        LIGHTS,
        VBOCOUNT,
    };
    GLuint Vao = { 0 };
    GLuint Vbo[VBOCOUNT] = { 0 };
	
	static GLuint VboVertices, VboUvs;

	size_t hash;
    Vector sectionPos;

    RendererSection(const RendererSection &other) = delete;
public:
    RendererSection(RendererSectionData data);

    RendererSection(RendererSection &&other);

	~RendererSection();

	void Render(RenderState &renderState);

    Vector GetPosition();

    size_t GetHash();

    size_t numOfFaces;

    friend void swap(RendererSection &lhs, RendererSection &rhs);
};