#pragma once

#include <glm/glm.hpp>

#include "Utility.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "AssetManager.hpp"

class RendererSky {
    GLuint VboVert, VboUv, Vao;
public:
    RendererSky();
    ~RendererSky();
    void Render(RenderState &renderState);
};