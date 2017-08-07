#pragma once

#include "RendererSection.hpp"
#include "GameState.hpp"
#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

class RendererWorld: public Renderer {
    GameState *gs;
    std::mutex sectionsMutex;
    std::map<Vector, RendererSection> sections;
    EventListener listener;
    Shader *shader;
    std::thread resourceLoader;
    void LoadedSectionController();
    bool isRunning = true;
public:
	RendererWorld(GameState* ptr);
	~RendererWorld();

    void Render(RenderState& renderState) override;
    void PrepareResources() override;
    void PrepareRender() override;
    bool IsNeedResourcesPrepare() override;

    double MaxRenderingDistance;
};