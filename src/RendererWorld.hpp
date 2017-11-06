#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <queue>
#include <memory>

#include "RendererSection.hpp"
#include "RendererEntity.hpp"
#include "RendererSky.hpp"

class Frustum;
class GameState;
class Texture;
class Shader;
class EventListener;
class RenderState;
class RendererSectionData;

class RendererWorld {
    //General
    GameState *gs;
    std::unique_ptr<EventListener> listener;
    size_t numOfWorkers;
    size_t currentWorker = 0;
    std::vector<std::thread> workers;
    void WorkerFunction(size_t WorkerId);
    bool isRunning = true;
    std::mutex isParsingMutex;
    std::map<Vector, bool> isParsing;
    //Blocks
    std::mutex renderDataMutex;
    std::queue<std::unique_ptr<RendererSectionData>> renderData;
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
	RendererWorld(GameState* ptr);
	~RendererWorld();

    void Render(RenderState& renderState);
    void PrepareRender();

    double MaxRenderingDistance;

    void Update(double timeToUpdate);

    GameState *GameStatePtr();

    int culledSections = 0;
};