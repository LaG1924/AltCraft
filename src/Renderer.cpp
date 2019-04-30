#include "Renderer.hpp"

#include <optick.h>

void RenderState::SetActiveVao(GLuint Vao) {
	OPTICK_EVENT();
    glBindVertexArray(Vao);
    ActiveVao = Vao;
}

void RenderState::SetActiveShader(GLuint Shader) {
	if (Shader != ActiveShader) {
		glUseProgram(Shader);
		ActiveShader = Shader;
	}
}