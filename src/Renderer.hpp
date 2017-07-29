#pragma once

#include <GL/glew.h>

class RenderState {
	GLuint ActiveVao;
	GLuint ActiveShader;
public:
	void SetActiveVao(GLuint Vao);
	void SetActiveShader(GLuint Shader);
};

struct Renderer {
	virtual ~Renderer() = default;
	virtual void Render(RenderState& renderState) = 0;
	virtual void PrepareResources() = 0;
	virtual void PrepareRender() = 0;
};