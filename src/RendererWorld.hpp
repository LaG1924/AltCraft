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
    /*static const size_t numOfWorkers = 4;
    size_t currentWorker = 0;
    std::thread workers[numOfWorkers];
    void WorkerFunction(size_t WorkerId);*/
    std::thread resourceLoader;
    void LoadedSectionController();
    bool isRunning = true;
    //Blocks
    std::vector<Vector> renderList;
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

    std::mutex renderDataMutex;
    std::queue<RendererSectionData> renderData;
};