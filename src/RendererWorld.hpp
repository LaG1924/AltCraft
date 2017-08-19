#pragma once

#include "RendererSection.hpp"
#include "RendererEntity.hpp"
#include "GameState.hpp"
#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

class RendererWorld: public Renderer {
    //General
    std::shared_ptr<GameState> gs;
    EventListener listener;
    size_t numOfWorkers;
    size_t currentWorker = 0;
    std::vector<std::thread> workers;
    void WorkerFunction(size_t WorkerId);
    bool isRunning = true;
    std::mutex isParsingMutex;
    std::map<Vector, bool> isParsing;
    //Blocks
    std::vector<Vector> renderList;
    std::mutex sectionsMutex;
    std::map<Vector, RendererSection> sections;
    Shader *blockShader;
    void UpdateAllSections(VectorF playerPos);
    //Entities
    Shader *entityShader;
    std::vector<RendererEntity> entities;
public:
	RendererWorld(std::shared_ptr<GameState> ptr);
	~RendererWorld();

    void Render(RenderState& renderState) override;
    void PrepareResources() override;
    void PrepareRender() override;
    bool IsNeedResourcesPrepare() override;

    double MaxRenderingDistance;

    void Update(double timeToUpdate);

    std::mutex renderDataMutex;
    std::queue<RendererSectionData> renderData;
};