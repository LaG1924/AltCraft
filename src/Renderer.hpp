#pragma once

#include <GL/glew.h>

class RenderState {
	GLuint ActiveVao = -1;
	GLuint ActiveShader = -1;
public:
	void SetActiveVao(GLuint Vao);
	void SetActiveShader(GLuint Shader);
    unsigned int WindowWidth, WindowHeight;
};

struct Renderer {
    virtual ~Renderer() = default;
    virtual void Render(RenderState& renderState) = 0;
    virtual void PrepareResources() = 0;
    virtual void PrepareRender() = 0;
    virtual bool IsNeedResourcesPrepare() = 0;
};