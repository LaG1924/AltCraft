#pragma once

#include "Renderer.hpp"
#include "Widget.hpp"
#include "Shader.hpp"

class RendererWidget {
    RootWidget *tree;
    
public:
    RendererWidget(RootWidget *widget);
    ~RendererWidget();

    void Render(RenderState &state);
};