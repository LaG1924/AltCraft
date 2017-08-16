#pragma once

#include "RendererSection.hpp"
#include "RendererEntity.hpp"
#include "GameState.hpp"
#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

class RendererWorld: public Renderer {
    //General
    GameState *gs;
    EventListener listener;
    std::thread resourceLoader;
    void LoadedSectionController();
    bool isRunning = true;
    //Blocks
    std::mutex sectionsMutex;
    std::map<Vector, RendererSection> sections;    
    Shader *blockShader;
    void RenderBlocks(RenderState& renderState);
    //Entities
    Shader *entityShader;
    std::vector<RendererEntity> entities;
    void RenderEntities(RenderState& renderState);
public:
	RendererWorld(GameState* ptr);
	~RendererWorld();

    void Render(RenderState& renderState) override;
    void PrepareResources() override;
    void PrepareRender() override;
    bool IsNeedResourcesPrepare() override;

    double MaxRenderingDistance;

    void Update();
};