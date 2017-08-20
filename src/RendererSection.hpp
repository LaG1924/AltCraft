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
};

class RendererSection {
	GLuint Vao, VboTextures, VboModels, VboColors, VboLights;
	
	static GLuint VboVertices, VboUvs;
	static std::map<GLuint, int> refCounterVbo;
	static std::map<GLuint, int> refCounterVao;

	size_t hash;
    Vector sectionPos;
public:
    RendererSection(RendererSectionData data);
	RendererSection(const RendererSection &other);
	~RendererSection();

	void Render(RenderState &renderState);

    Vector GetPosition();

    size_t GetHash();

    size_t numOfFaces = 0;
};