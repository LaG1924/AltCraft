#pragma once

#include "RendererSection.hpp"
#include "RendererEntity.hpp"
#include "RendererSky.hpp"
#include "GameState.hpp"
#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

class Frustum;

class RendererWorld {
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
    std::mutex renderDataMutex;
    std::queue<RendererSectionData> renderData;
    std::vector<Vector> renderList;
    std::mutex sectionsMutex;
    std::map<Vector, RendererSection> sections;
    Shader *blockShader;
    void UpdateAllSections(VectorF playerPos);
    std::unique_ptr<Frustum> frustum;
    //Entities
    Shader *entityShader;
    std::vector<RendererEntity> entities;
    //Sky
    Texture *skyTexture;
    Shader *skyShader;
    RendererSky rendererSky;
public:
	RendererWorld(std::shared_ptr<GameState> ptr);
	~RendererWorld();

    void Render(RenderState& renderState);
    void PrepareRender();

    double MaxRenderingDistance;

    void Update(double timeToUpdate);

    GameState *GameStatePtr();

    int culledSections = 0;
};