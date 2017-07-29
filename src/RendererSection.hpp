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

class RendererSection : Renderer {
	Vector sectionPosition;
	World *world;
	GLuint Vao, VboTextures, VboModels, VboColors;
	std::vector<glm::mat4> models;
	std::vector<glm::vec4> textures;
	std::vector<glm::vec3> colors;

	static GLuint VboVertices, VboUvs;
	static std::map<GLuint, int> refCounterVbo;
	static std::map<GLuint, int> refCounterVao;

	size_t numOfFaces = 0;

	bool isEnabled = true;

	size_t hash = 0;
public:
	RendererSection(World *world, Vector position);
	RendererSection(const RendererSection &other);
	~RendererSection() override;

	void Render(RenderState &renderState) override;

	void PrepareResources() override;

	void PrepareRender() override;

	void SetEnabled(bool isEnabled);

	Section *GetSection();

	bool IsNeedUpdate();
};